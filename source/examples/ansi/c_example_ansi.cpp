#include "Arduino.h"

#include "rcore/c_str.h"
#include "rdebug/c_telnet.h"
#include "rdebug/c_escape_codes.h"

#define SERIAL_SPEED  9600
#define WIFI_SSID     "MY SSID"
#define WIFI_PASSWORD "MY PASS"

IPAddress ip;
uint16_t  port = 23;

ndebug::telnet_t* gTelnet = nullptr;

void setupSerial(long speed, str_t const& msg)
{
    Serial.begin(speed);
    while (!Serial) {}
    delay(200);
    Serial.println();
    Serial.println();
    if (str_is_empty(msg))
        Serial.println(msg.m_const);
}

bool isConnected() { return (WiFi.status() == WL_CONNECTED); }

bool connectToWiFi(const char* ssid, const char* password, int max_tries = 20, int pause = 500)
{
    int i = 0;
    WiFi.mode(WIFI_STA);
#if defined(ARDUINO_ARCH_ESP8266)
    WiFi.forceSleepWake();
    delay(200);
#endif
    WiFi.begin(ssid, password);
    do
    {
        delay(pause);
        Serial.print(".");
    } while (!isConnected() || i++ < max_tries);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    return isConnected();
}

void errorMsg(String error, bool restart = true)
{
    Serial.println(error);
    if (restart)
    {
        Serial.println("Rebooting now...");
        delay(2000);
        ESP.restart();
        delay(2000);
    }
}

void setupTelnet()
{
    gTelnet = telnetInit();

    // passing on functions for various telnet events
    telnetSetOnConnect(gTelnet, onTelnetConnect);
    telnetSetOnConnectionAttempt(gTelnet, onTelnetConnectionAttempt);
    telnetSetOnReconnect(gTelnet, onTelnetReconnect);
    telnetSetOnDisconnect(gTelnet, onTelnetDisconnect);
    telnetSetOnInputReceived(gTelnet, onTelnetInput);

    Serial.print("- Telnet: ");
    if (telnetBegin(gTelnet, port))
    {
        Serial.println("running");
    }
    else
    {
        Serial.println("error.");
        errorMsg("Will reboot...");
    }
}

// (optional) callback functions for telnet events
void onTelnetConnect(str_t const& ip)
{
    Serial.print("- Telnet: ");
    Serial.print(ip.m_const);
    Serial.println(" connected");

    telnetPrint(gTelnet, ansi.cls());
    telnetPrint(gTelnet, ansi.home());
    telnetPrint(gTelnet, ansi.setFG(ANSI_BRIGHT_WHITE));
    telnetPrintln(gTelnet, "\nWelcome " + telnetGetIP(gTelnet));
    telnetPrintln(gTelnet, "(Use ^] + q  to disconnect.)");
    telnetPrint(gTelnet, ansi.reset());
}

void onTelnetDisconnect(str_t const& ip)
{
    Serial.print("- Telnet: ");
    Serial.print(ip.m_const);
    Serial.println(" disconnected");
}

void onTelnetReconnect(str_t const& ip)
{
    Serial.print("- Telnet: ");
    Serial.print(ip.m_const);
    Serial.println(" reconnected");
}

void onTelnetConnectionAttempt(str_t const& ip)
{
    Serial.print("- Telnet: ");
    Serial.print(ip.m_const);
    Serial.println(" tried to connected");
}

void onTelnetInput(str_t const& str)
{
    // checks for a certain command
    if (ncore::str_eq(str, "ping"))
    {
        telnetPrint(gTelnet, ansi.setFG(ANSI_BRIGHT_WHITE));
        telnetPrintln(gTelnet, "> pong");
        telnetPrint(gTelnet, ansi.reset());
        Serial.println("- Telnet: pong");
        // disconnect the client
    }
    else if (ncore::str_eq(str, "bye"))
    {
        telnetPrint(gTelnet, ansi.setFG(ANSI_BRIGHT_WHITE));
        telnetPrintln(gTelnet, "> disconnecting you...");
        telnetPrint(gTelnet, ansi.reset());
        telnetDisconnectClient(gTelnet);
    }
}

void setup()
{
    setupSerial(SERIAL_SPEED, "Telnet Test");

    Serial.print("- Wifi: ");
    connectToWiFi(WIFI_SSID, WIFI_PASSWORD);

    if (isConnected())
    {
        ip = WiFi.localIP();
        Serial.println();
        Serial.print("- Telnet: ");
        Serial.print(ip);
        Serial.print(":");
        Serial.println(port);
        setupTelnet();
    }
    else
    {
        Serial.println();
        errorMsg(ncore::str_const("Error connecting to WiFi"));
    }
}

void loop()
{
    telnetLoop(gTelnet);

    // send serial input to telnet as output
    if (Serial.available())
    {
        telnetPrint(gTelnet, Serial.read());
    }
}
