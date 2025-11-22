#ifndef __RDNO_DEBUG_TELNET_BASE_H__
#define __RDNO_DEBUG_TELNET_BASE_H__

#define ASCII_BELL             7
#define ASCII_BACKSPACE        8
#define ASCII_LF               10
#define ASCII_CR               13
#define KEEP_ALIVE_INTERVAL_MS 1000
#define MAX_ERRORS_ON_WRITE    3

#include "ccore/c_va_list.h"
#include "rdno_core/c_str.h"
#include "rdno_telnet/c_macros.h"

namespace ncore
{
    typedef void (*telnet_func_cb)(ncore::str_t const& str);

    void telnetSetOnConnect(telnet_func_cb f);
    void telnetSetOnConnectionAttempt(telnet_func_cb f);
    void telnetSetOnReconnect(telnet_func_cb f);
    void telnetSetOnDisconnect(telnet_func_cb f);
    void telnetSetOnInputReceived(telnet_func_cb f);

    bool telnetBegin(u16 port = 23, bool checkConnection = true);
    void telnetLoop();

    void telnetPrint(char c);
    void telnetPrint(const char* msg);

    void telnetWrite(byte c);
    void telnetWrite(const byte* data, s32 length);

    char telnetReadChar();
    s32  telnetReadChars(char* buffer, s32 length);
    s32  telnetReadBytes(byte* buffer, s32 length);

    void telnetPrintf_(const char* format, va_t* args, s32 argc);
    // Variadic template wrapper for telnetPrintf
    template <typename... Args>
    void telnetPrintf(const char* format, Args&&... _args)
    {
        const va_t argv[] = {_args...};
        const s32  argc   = sizeof(argv) / sizeof(argv[0]);
        telnetPrintf_(telnet, format, (va_t*)argv, argc);
    }

}  // namespace ncore
#endif  // __RDNO_DEBUG_TELNET_BASE_H__