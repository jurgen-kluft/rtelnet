#ifndef __RDNO_DEBUG_TELNET_H__
#define __RDNO_DEBUG_TELNET_H__

#include "rdno_telnet/c_telnet_base.h"

namespace ncore
{
    void telnetPrintln();
    s32  telnetPrintf(const char* format, ...);

    bool telnetIsLineModeSet();
    void telnetSetLineMode(bool value = true);

    char telnetGetNewlineCharacter();
    void telnetSetNewlineCharacter(char value = '\n');

    void telnetHandleInput();
}  // namespace ncore
#endif
