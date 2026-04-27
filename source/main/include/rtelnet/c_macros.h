#pragma once

#ifndef DebugMacros_h
#    define DebugMacros_h

/* ------------------------------------------------- */

#    ifndef DEBUG_SERIAL
#        define DEBUG_SERIAL_PRINT(x)   Serial.print(x)
#        define DEBUG_SERIAL_PRINTLN(x) Serial.println(x)
#    endif
#    ifndef DEBUG_TELNET
#        define DEBUG_TELNET_PRINT(x)   telnetPrint((x))
#        define DEBUG_TELNET_PRINTLN(x) telnetPrintln((x))
#    endif
#    ifndef DEBUG_PREFIX
#        define DEBUG_PREFIX "[DEBUG]\t"
#    endif

/* ------------------------------------------------- */

#    ifndef DEBUG_ON
#        define DEBUG_INFO
#        define DEBUG_MSG(x)
#        define DEBUG_VAR(...)
#        define DEBUG_WHERE
#    else
#        if DEBUG_USE_SERIAL
#            define DEBUG_SER_INFO                \
                DEBUG_SERIAL_PRINT(DEBUG_PREFIX); \
                DEBUG_SERIAL_PRINTLN("Compiled " __DATE__ ", " __TIME__)
#            define DEBUG_SER_MSG(x)              \
                DEBUG_SERIAL_PRINT(DEBUG_PREFIX); \
                DEBUG_SERIAL_PRINTLN(x)
#            define DEBUG_SER_VAR(...)                     \
                DEBUG_SERIAL_PRINT(DEBUG_PREFIX);          \
                DEBUG_SERIAL_PRINT(F(#__VA_ARGS__ " = ")); \
                DEBUG_SERIAL_PRINTLN(__VA_ARGS__)
#            define DEBUG_SER_WHERE                      \
                DEBUG_SERIAL_PRINT(DEBUG_PREFIX);        \
                DEBUG_SERIAL_PRINT(F(__FILE__ " - "));   \
                DEBUG_SERIAL_PRINT(__PRETTY_FUNCTION__); \
                DEBUG_SERIAL_PRINT(": ");                \
                DEBUG_SERIAL_PRINTLN(__LINE__)
#        else
#            define DEBUG_SER_INFO
#            define DEBUG_SER_MSG(x)
#            define DEBUG_SER_VAR(...)
#            define DEBUG_SER_WHERE
#        endif

#        if DEBUG_USE_TELNET
#            define DEBUG_TEL_INFO                \
                DEBUG_TELNET_PRINT(DEBUG_PREFIX); \
                DEBUG_TELNET_PRINTLN("Compiled " __DATE__ ", " __TIME__)
#            define DEBUG_TEL_MSG(x)              \
                DEBUG_TELNET_PRINT(DEBUG_PREFIX); \
                DEBUG_TELNET_PRINTLN(x)
#            define DEBUG_TEL_VAR(...)                     \
                DEBUG_TELNET_PRINT(DEBUG_PREFIX);          \
                DEBUG_TELNET_PRINT(F(#__VA_ARGS__ " = ")); \
                DEBUG_TELNET_PRINTLN(String(__VA_ARGS__))
#            define DEBUG_TEL_WHERE                      \
                DEBUG_TELNET_PRINT(DEBUG_PREFIX);        \
                DEBUG_TELNET_PRINT(F(__FILE__ " - "));   \
                DEBUG_TELNET_PRINT(__PRETTY_FUNCTION__); \
                DEBUG_TELNET_PRINT(": ");                \
                DEBUG_TELNET_PRINTLN(String(__LINE__))
#        else
#            define DEBUG_TEL_INFO
#            define DEBUG_TEL_MSG(x)
#            define DEBUG_TEL_VAR(...)
#            define DEBUG_TEL_WHERE
#        endif

#        define DEBUG_INFO  \
            DEBUG_SER_INFO; \
            DEBUG_TEL_INFO
#        define DEBUG_MSG(x)  \
            DEBUG_SER_MSG(x); \
            DEBUG_TEL_MSG(x)
#        define DEBUG_VAR(...)          \
            DEBUG_SER_VAR(__VA_ARGS__); \
            DEBUG_TEL_VAR(__VA_ARGS__)
#        define DEBUG_WHERE  \
            DEBUG_SER_WHERE; \
            DEBUG_TEL_WHERE
#    endif

/* ------------------------------------------------- */
#endif
/* ------------------------------------------------- */
