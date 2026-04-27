#ifndef __ARDUINO_DEBUG_TELNET_BASE_H__
#define __ARDUINO_DEBUG_TELNET_BASE_H__

#define ASCII_BELL             7
#define ASCII_BACKSPACE        8
#define ASCII_LF               10
#define ASCII_CR               13
#define KEEP_ALIVE_INTERVAL_MS 1000
#define MAX_ERRORS_ON_WRITE    3

#include "ccore/c_va_list.h"
#include "rcore/c_str.h"

#include "rtelnet/c_macros.h"

namespace ncore
{
    typedef void (*telnet_func_cb)(IPAddress_t const& ip);
    typedef void (*telnet_input_cb)(str_t const& input);

    void telnetSetOnConnect(telnet_func_cb f);
    void telnetSetOnConnectionAttempt(telnet_func_cb f);
    void telnetSetOnReconnect(telnet_func_cb f);
    void telnetSetOnDisconnect(telnet_func_cb f);
    void telnetSetOnInputReceived(telnet_input_cb f);

    bool telnetBegin(u16 port = 23, bool checkConnection = true);
    void telnetLoop();

    IPAddress_t telnetGetIP();
    IPAddress_t telnetGetLastAttemptIP();

    void telnetPrint(char c);
    void telnetPrint(const char* msg);
    void telnetPrintln();

    size_t telnetWrite(byte c);
    size_t telnetWrite(const byte* data, size_t length);

    s32  telnetReadBytes(byte* buffer, s32 length);

    size_t telnetPrintf_(const char* format, va_t* args, s32 argc);
    // Variadic template wrapper for telnetPrintf
    template <typename... Args>
    void telnetPrintf(const char* format, Args&&... _args)
    {
        const va_t argv[] = {_args...};
        const s32  argc   = sizeof(argv) / sizeof(argv[0]);
        telnetPrintf_(format, (va_t*)argv, argc);
    }

    bool telnetIsLineModeSet();
    void telnetSetLineMode(bool value = true);

    char telnetGetNewlineCharacter();
    void telnetSetNewlineCharacter(char value = '\n');

    void telnetHandleInput();

}  // namespace ncore
#endif  // __ARDUINO_DEBUG_TELNET_BASE_H__