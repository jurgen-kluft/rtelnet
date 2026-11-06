#ifndef __RTELNET_ESCAPE_CODES_H__
#define __RTELNET_ESCAPE_CODES_H__

#include "rcore/c_str.h"

#define ANSI_BLACK          0
#define ANSI_RED            1
#define ANSI_GREEN          2
#define ANSI_YELLOW         3
#define ANSI_BLUE           4
#define ANSI_MAGENTA        5
#define ANSI_CYAN           6
#define ANSI_WHITE          7
#define ANSI_GREY           60
#define ANSI_BRIGHT_RED     61
#define ANSI_BRIGHT_GREEN   62
#define ANSI_BRIGHT_YELLOW  63
#define ANSI_BRIGHT_BLUE    64
#define ANSI_BRIGHT_MAGENTA 65
#define ANSI_BRIGHT_CYAN    66
#define ANSI_BRIGHT_WHITE   67

namespace ncore
{
    namespace nescape_codes
    {
        static const char* home();
        static const char* cls();
        static const char* clearLine();
        static const char* clearEoLine();

        static void cursorXY(str_t& str, int x, int y);
        static void cursorUp(str_t& str, int x);
        static void cursorDown(str_t& str, int x);
        static void cursorLeft(str_t& str, int x);
        static void cursorRight(str_t& str, int x);

        static const char* setBG(int color);
        static const char* setFG(int color);

        static void bold(str_t& out_str, const char* str);
        static void italic(str_t& out_str, const char* str);
        static void underline(str_t& out_str, const char* str);
        static void blink(str_t& out_str, const char* str);
        static void inverse(str_t& out_str, const char* str);

        static const char* showCursor(bool blink);
        static const char* reset();

        static const char* prefix();
        static void        prefixAndNumberAndValue(str_t& str, int x, char v);
        static void        setAttribute(str_t& str, int a);
    }  // namespace nescape_codes
};  // namespace ncore

#endif // __RTELNET_ESCAPE_CODES_H__
