#include "rtelnet/c_telnet.h"

#if defined(TARGET_ARDUINO)

#include "Arduino.h"
#if defined(TARGET_ESP32)
#    include "WiFi.h"
#    include "MacAddress.h"
// Include ETH.h for Ethernet support on ESP32 when available
#    if __has_include(<ETH.h>)
#        include <ETH.h>
#    endif
#elif defined(TARGET_ESP8266)
#    include "ESP8266WiFi.h"
#endif

#include "ccore/c_printf.h"
#include "rcore/c_network.h"
#include "rcore/c_str.h"
#include "rcore/c_timer.h"

using TCPClient = WiFiClient;
using TCPServer = WiFiServer;

namespace ncore
{
    bool gIsIPSet(IPAddress ip)
    {
#if defined(ARDUINO_ARCH_ESP8266)
        return ip.isSet();
#else
        // this works for ESP32, hopefully for others as well
        return ip.toString() != "0.0.0.0";
#endif
    }

    struct telnet_t
    {
        telnet_t()
            : m_server(23)  // constructor to initialize m_server
        {
        }

        TCPServer m_server;
        TCPClient m_client;

        IPAddress_t m_ip;  // max length of IPv4 string is 15 + null terminator
        IPAddress_t m_attempt_ip;

        s32  m_input_pos;
        char m_input[256];

        char m_newline_character;
        bool m_connected;  // needed because I cannot do "client = NULL"
        bool m_line_mode;

        u16 m_server_port;
        s32 m_keep_alive_interval;
        u64 m_last_status_check;
        u32 m_failedWrites;

        telnet_func_cb  m_on_connect;
        telnet_func_cb  m_on_reconnect;
        telnet_func_cb  m_on_disconnect;
        telnet_func_cb  m_on_connection_attempt;
        telnet_input_cb m_on_input;
    };

    telnet_t gTelnet;

    bool telnetBegin(u16 port, bool checkConnection)
    {
        gTelnet.m_input_pos = 0;
        gTelnet.m_input[256];

        gTelnet.m_newline_character = '\n';
        gTelnet.m_connected         = false;  // needed because I cannot do "client = NULL"
        gTelnet.m_line_mode         = true;

        gTelnet.m_server_port         = 23;
        gTelnet.m_keep_alive_interval = KEEP_ALIVE_INTERVAL_MS;
        gTelnet.m_last_status_check;
        gTelnet.m_failedWrites = 0;

        gTelnet.m_on_connect            = NULL;
        gTelnet.m_on_reconnect          = NULL;
        gTelnet.m_on_disconnect         = NULL;
        gTelnet.m_on_connection_attempt = NULL;
        gTelnet.m_on_input              = NULL;

        if (checkConnection)
        {
            bool isConnected = false;

            // Check WiFi connection (supported on both ESP32 and ESP8266)
            if (WiFi.status() == WL_CONNECTED || gIsIPSet(WiFi.softAPIP()))
            {
                isConnected = true;
            }

#if defined(TARGET_ESP32)
// Check Ethernet connection for ESP32 (ETH.h support)
// ETH.h defines ETH object when Ethernet is available
#    ifdef ETH_H
            if (!isConnected && ETH.linkUp())
            {
                isConnected = true;
            }
#    endif

// Alternative check using event bits (for advanced ESP32 setups)
#    ifdef ETH_CONNECTED_BIT
            if (!isConnected && (WiFiGenericClass::getStatusBits() & ETH_CONNECTED_BIT))
            {
                isConnected = true;
            }
#    endif
#elif defined(TARGET_ESP8266)
            // ESP8266 doesn't have native Ethernet support
            // But users might use SPI-based Ethernet libraries like EthernetENC, W5500, etc.
            // These typically create a custom client that works with existing WiFi infrastructure
            // For now, we rely on WiFi checks only for ESP8266
            // Users with SPI Ethernet modules should disable connection check: begin(port, false)
#endif

            if (!isConnected)
            {
                return false;
            }
        }
        gTelnet.m_server_port = port;
        gTelnet.m_server      = TCPServer(port);
        gTelnet.m_server.begin();
        gTelnet.m_server.setNoDelay(true);
        return true;
    }

    void telnetOnFailedWrite();
    void telnetOnSuccessfullyWrite();
    void telnetEmptyClientStream();

    void telnetProcessClientConnection();
    void telnetPerformKeepAliveCheck();
    void telnetHandleClientInput();

    void telnetHandleExistingConnection(TCPClient& newClient);

    void telnetConnectClient(TCPClient& c, bool triggerEvent = true);
    void telnetDisconnectClient(bool triggerEvent = true);

    bool telnetIsConnected();

    void telnetLoop()
    {
        telnetProcessClientConnection();
        telnetPerformKeepAliveCheck();
        telnetHandleClientInput();
    }

    void telnetProcessClientConnection()
    {
        if (!gTelnet.m_server.hasClient())
            return;

        TCPClient newClient = gTelnet.m_server.accept();
        if (!gTelnet.m_connected)
        {
            telnetConnectClient(newClient);
        }
        else
        {
            telnetHandleExistingConnection(newClient);
        }
    }

    void telnetHandleReconnection(TCPClient& newClient, const IPAddress_t& attemptedIp)
    {
        telnetDisconnectClient(false);
        telnetConnectClient(newClient, false);
        if (gTelnet.m_on_reconnect)
            gTelnet.m_on_reconnect(attemptedIp);
    }

    void telnetNotifyConnectionAttempt(const IPAddress_t& attemptedIp)
    {
        if (gTelnet.m_on_connection_attempt)
            gTelnet.m_on_connection_attempt(attemptedIp);
    }

    void telnetHandleExistingConnection(TCPClient& newClient)
    {
        IPAddress_t attemptedIp = newClient.remoteIP();
        if (!telnetIsConnected())
        {
            telnetDisconnectClient();
            return;
        }

        if (attemptedIp == gTelnet.m_ip)
        {
            telnetHandleReconnection(newClient, attemptedIp);
        }
        else
        {
            telnetNotifyConnectionAttempt(attemptedIp);
        }
    }

    bool telnetDoKeepAliveCheckNow()
    {
        u64 currentTime = ntimer::millis();
        u64 timeElapsed = currentTime - gTelnet.m_last_status_check;

        if (timeElapsed >= static_cast<u64>(gTelnet.m_keep_alive_interval))
        {
            gTelnet.m_last_status_check = currentTime;
            return true;
        }
        return false;
    }

    void telnetPerformKeepAliveCheck()
    {
        if (telnetDoKeepAliveCheckNow() && gTelnet.m_connected && !telnetIsConnected())
        {
            telnetDisconnectClient();
        }
    }

    void telnetHandleClientInput()
    {
        if (gTelnet.m_on_input && gTelnet.m_client && gTelnet.m_client.available())
        {
            telnetHandleInput();
        }
    }

    void telnetSetKeepAliveInterval(s32 ms) { gTelnet.m_keep_alive_interval = ms; }
    s32  telnetGetKeepAliveInterval() { return gTelnet.m_keep_alive_interval; }

    void telnetFlush()
    {
        if (!gTelnet.m_client || !telnetIsConnected())
        {
            return;
        }

        // only the ESP8266 has a "bool flush()" method
        // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFiClient.cpp#L306

#ifdef TARGET_ESP8266
        if (!gTelnet.m_client.flush(telnetGetKeepAliveInterval()))
        {
            telnetOnFailedWrite();
        }
        else
        {
            telnetOnSuccessfullyWrite();
        }
#elif defined(TARGET_ESP32)
        // ESP32 implementation with timeout to ensure data transmission
        gTelnet.m_client.flush();
        u64    startTime         = ntimer::millis();
        size_t previousAvailable = 0;
        size_t stableCount       = 0;

        // Wait for data to be transmitted by monitoring available write buffer
        while (ntimer::millis() - startTime < static_cast<u64>(telnetGetKeepAliveInterval()))
        {
            size_t currentAvailable = gTelnet.m_client.availableForWrite();

            // Check if buffer has been stable (indicating transmission complete)
            if (currentAvailable == previousAvailable && currentAvailable > 0)
            {
                stableCount++;
                if (stableCount >= 10)
                {  // Buffer stable for 10ms
                    telnetOnSuccessfullyWrite();
                    return;
                }
            }
            else
            {
                stableCount = 0;
            }

            previousAvailable = currentAvailable;
            ntimer::delay(1);
        }

        // Timeout occurred - data may not have been fully transmitted
        telnetOnFailedWrite();
#else
        gTelnet.m_client.flush();
#endif
    }

    size_t telnetWrite(byte data)
    {
        if (!gTelnet.m_client || !telnetIsConnected())
        {
            return 0;
        }

        size_t written = gTelnet.m_client.write(data);
        if (!written)
        {
            telnetOnFailedWrite();
        }
        else
        {
            telnetOnSuccessfullyWrite();
        }
        return written;
    }

    size_t telnetWrite(const byte* data, size_t size)
    {
        if (!gTelnet.m_client || !telnetIsConnected())
        {
            return 0;
        }

        size_t written = gTelnet.m_client.write(data, size);
        if (written != size)
        {
            telnetOnFailedWrite();
        }
        else
        {
            telnetOnSuccessfullyWrite();
        }

        return written;
    }

    void telnetConnectClient(TCPClient& c, bool triggerEvent)
    {
        gTelnet.m_client = c;
        gTelnet.m_ip     = gTelnet.m_client.remoteIP();
        gTelnet.m_client.setNoDelay(true);
        gTelnet.m_client.setTimeout(telnetGetKeepAliveInterval());
        if (triggerEvent && gTelnet.m_on_connect != NULL)
            gTelnet.m_on_connect(gTelnet.m_ip);
        telnetEmptyClientStream();
        gTelnet.m_connected = true;
    }

    void telnetDisconnectClient(bool triggerEvent)
    {
        telnetEmptyClientStream();
        gTelnet.m_client.stop();
        if (triggerEvent && gTelnet.m_on_disconnect != NULL)
            gTelnet.m_on_disconnect(gTelnet.m_ip);
        gTelnet.m_ip        = IPAddress_t();
        gTelnet.m_connected = false;
    }

    // helper method, as ESP32's IPAddress has no isSet() method

    bool telnetIsIPSet(IPAddress const& ip)
    {
#if defined(ARDUINO_ARCH_ESP8266)
        return gTelnet.m_ip.isSet();
#else
        // this works for ESP32, hopefully for others as well
        // return gTelnet.m_ip.toString() != "0.0.0.0";
        return gTelnet.m_ip[0] != 0 || gTelnet.m_ip[1] != 0 || gTelnet.m_ip[2] != 0 || gTelnet.m_ip[3] != 0;
#endif
    }

    void telnetStop(bool disconnectClient)
    {
        if (disconnectClient && gTelnet.m_connected)
        {
            telnetDisconnectClient();
        }
        gTelnet.m_server.stop();
    }

    bool telnetIsConnected()
    {
        bool res;
#if defined(ARDUINO_ARCH_ESP8266)
        res = gTelnet.m_client.status() == ESTABLISHED;
#elif defined(ARDUINO_ARCH_ESP32)
        res = gTelnet.m_client.connected();
#endif
        return res;
    }

    IPAddress_t telnetGetIP() { return gTelnet.m_ip; }
    IPAddress_t telnetGetLastAttemptIP() { return gTelnet.m_attempt_ip; }

    void telnetOnFailedWrite()
    {
        gTelnet.m_failedWrites++;
        if (gTelnet.m_failedWrites >= MAX_ERRORS_ON_WRITE)
        {
            gTelnet.m_failedWrites = 0;
            telnetDisconnectClient();
        }
    }

    void telnetOnSuccessfullyWrite()
    {
        if (gTelnet.m_failedWrites > 0)
        {
            gTelnet.m_failedWrites = 0;
        }
    }

    void telnetEmptyClientStream()
    {
        telnetFlush();
        ntimer::delay(50);
        while (gTelnet.m_client.available())
        {
            gTelnet.m_client.read();
        }
    }

    void telnetSetOnConnect(telnet_func_cb f) { gTelnet.m_on_connect = f; }
    void telnetSetOnConnectionAttempt(telnet_func_cb f) { gTelnet.m_on_connection_attempt = f; }
    void telnetSetOnReconnect(telnet_func_cb f) { gTelnet.m_on_reconnect = f; }
    void telnetSetOnDisconnect(telnet_func_cb f) { gTelnet.m_on_disconnect = f; }
    void telnetSetOnInputReceived(telnet_input_cb f) { gTelnet.m_on_input = f; }

    TCPClient& telnetGetClient() { return gTelnet.m_client; }

    // -----------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------
    // c_telnet.h functions

    void telnetHandleInput()
    {
        char c = gTelnet.m_client.read();

        // collect string
        if (gTelnet.m_line_mode)
        {
            if (c != gTelnet.m_newline_character)
            {
                if (c >= 32 && c < 127)
                {
                    gTelnet.m_input[gTelnet.m_input_pos++] = c;
                }
                // EOL -> send input
            }
            else
            {
                str_t input = str_const_n(gTelnet.m_input, gTelnet.m_input_pos);
                gTelnet.m_on_input(input);
                gTelnet.m_input_pos = 0;
                gTelnet.m_input[0]  = '\0';
            }
            // send individual characters
        }
        else
        {
            if (gTelnet.m_input_pos > 0)
            {
                gTelnet.m_input[gTelnet.m_input_pos++] = c;
                str_t input                            = str_const_n(gTelnet.m_input, gTelnet.m_input_pos);
                gTelnet.m_on_input(input);
                gTelnet.m_input_pos = 0;
                gTelnet.m_input[0]  = '\0';
            }
            else
            {
                gTelnet.m_input[gTelnet.m_input_pos++] = c;
                str_t input                            = str_const_n(gTelnet.m_input, gTelnet.m_input_pos);
                gTelnet.m_on_input(input);
                gTelnet.m_input_pos = 0;
                gTelnet.m_input[0]  = '\0';
            }
        }
    }

    void telnetPrint(char c)
    {
        if (gTelnet.m_client && telnetIsConnected())
        {
            if (!gTelnet.m_client.write(c))
            {
                telnetOnFailedWrite();
            }
            else
            {
                telnetOnSuccessfullyWrite();
            }
        }
    }

    void telnetPrint(const char* msg)
    {
        if (gTelnet.m_client && telnetIsConnected())
        {
            if (!gTelnet.m_client.print(msg))
            {
                telnetOnFailedWrite();
            }
            else
            {
                telnetOnSuccessfullyWrite();
            }
        }
    }

    void telnetPrintln()
    {
        if (gTelnet.m_client && telnetIsConnected())
        {
            if (!gTelnet.m_client.println())
            {
                telnetOnFailedWrite();
            }
            else
            {
                telnetOnSuccessfullyWrite();
            }
        }
    }

    s32 telnetReadBytes(byte* buffer, s32 length)
    {
        if (gTelnet.m_client && telnetIsConnected())
        {
            return gTelnet.m_client.readBytes((char*)buffer, length);
        }
        return 0;
    }

    size_t telnetPrintf_(const char* format, va_t* args, s32 argc)
    {
        if (!gTelnet.m_client || !telnetIsConnected())
            return 0;

        char loc_buf[64];
        s32  len = vsnprintf_(loc_buf, sizeof(loc_buf), format, args, argc);
        if (len < 0)
            return 0;

        if (len >= (s32)sizeof(loc_buf))
        {
            char* temp = (char*)malloc(len + 1);
            if (temp == nullptr)
                return 0;

            vsnprintf_(temp, len + 1, format, args, argc);
            len = telnetWrite((u8*)temp, len);

            free(temp);
        }
        else
        {
            len = telnetWrite((u8*)loc_buf, len);
        }

        return len;
    }

    bool telnetIsLineModeSet() { return gTelnet.m_line_mode; }
    void telnetSetLineMode(bool value) { gTelnet.m_line_mode = value; }
    char telnetGetNewlineCharacter() { return gTelnet.m_newline_character; }
    void telnetSetNewlineCharacter(char value) { gTelnet.m_newline_character = value; }

}  // namespace ncore

#endif