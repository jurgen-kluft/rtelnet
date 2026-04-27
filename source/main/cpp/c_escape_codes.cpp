#include "rtelnet/c_escape_codes.h"

namespace ncore
{
    namespace nescape_codes
    {
        const char* cls() { return "\033[1J"; }
        const char* home() { return "\033[H"; }
        const char* clearLine() { return "\033[2K"; }
        const char* clearEoLine() { return "\033[K"; }

        void cursorXY(str_t& str, int x, int y)
        {
            str_append(str, "\033[");
            to_str(str, y);
            str_append(str, ";");
            to_str(str, x);
            str_append(str, "H");
        }
        void cursorUp(str_t& str, int x) { return prefixAndNumberAndValue(str, x, 'A'); }
        void cursorDown(str_t& str, int x) { return prefixAndNumberAndValue(str, x, 'B'); }
        void cursorRight(str_t& str, int x) { return prefixAndNumberAndValue(str, x, 'C'); }
        void cursorLeft(str_t& str, int x) { return prefixAndNumberAndValue(str, x, 'D'); }

        const char* setBG(int color) 
        { 
            switch (color)
            {
                case ANSI_BLACK:          return "\033[40m";
                case ANSI_RED:            return "\033[41m";
                case ANSI_GREEN:          return "\033[42m";
                case ANSI_YELLOW:         return "\033[43m";
                case ANSI_BLUE:           return "\033[44m";
                case ANSI_MAGENTA:        return "\033[45m";
                case ANSI_CYAN:           return "\033[46m";
                case ANSI_WHITE:          return "\033[47m";
                case ANSI_GREY:           return "\033[100m";
                case ANSI_BRIGHT_RED:     return "\033[101m";
                case ANSI_BRIGHT_GREEN:   return "\033[102m";
                case ANSI_BRIGHT_YELLOW:  return "\033[103m";
                case ANSI_BRIGHT_BLUE:    return "\033[104m";
                case ANSI_BRIGHT_MAGENTA: return "\033[105m";
                case ANSI_BRIGHT_CYAN:    return "\033[106m";
                case ANSI_BRIGHT_WHITE:   return "\033[107m";
                default:                  return "\033[49m"; // Default background
            }
        }
        const char* setFG(int color) 
        { 
            switch (color)
            {
                case ANSI_BLACK:          return "\033[30m";
                case ANSI_RED:            return "\033[31m";
                case ANSI_GREEN:          return "\033[32m";
                case ANSI_YELLOW:         return "\033[33m";
                case ANSI_BLUE:           return "\033[34m";
                case ANSI_MAGENTA:        return "\033[35m";
                case ANSI_CYAN:           return "\033[36m";
                case ANSI_WHITE:          return "\033[37m";
                case ANSI_GREY:           return "\033[90m";
                case ANSI_BRIGHT_RED:     return "\033[91m";
                case ANSI_BRIGHT_GREEN:   return "\033[92m";
                case ANSI_BRIGHT_YELLOW:  return "\033[93m";
                case ANSI_BRIGHT_BLUE:    return "\033[94m";
                case ANSI_BRIGHT_MAGENTA: return "\033[95m";
                case ANSI_BRIGHT_CYAN:    return "\033[96m";
                case ANSI_BRIGHT_WHITE:   return "\033[97m";
                default:                  return "\033[39m"; // Default foreground
            }
        }

        void bold(str_t& out_str, const char* str)
        {
            setAttribute(out_str, 1);
            str_append(out_str, str);
            setAttribute(out_str, 22);
        }
        void blink(str_t& out_str, const char* str)
        {
            setAttribute(out_str, 5);
            str_append(out_str, str);
            setAttribute(out_str, 25);
        }
        void italic(str_t& out_str, const char* str)
        {
            setAttribute(out_str, 3);
            str_append(out_str, str);
            setAttribute(out_str, 23);
        }
        void underline(str_t& out_str, const char* str)
        {
            setAttribute(out_str, 4);
            str_append(out_str, str);
            setAttribute(out_str, 24);
        }
        void inverse(str_t& out_str, const char* str)
        {
            setAttribute(out_str, 7);
            str_append(out_str, str);
            setAttribute(out_str, 27);
        }

        const char* showCursor(bool blink) { return blink ? "\033[?25h" : "\033[?25l"; }
        const char* reset() { return "\033[m"; }
        const char* prefix() { return "\033["; }

        void prefixAndNumberAndValue(str_t& str, int x, char v)
        {
            str_append(str, "\033[");
            to_str(str, x);
            str.m_ascii[str.m_end++] = v;
            str.m_ascii[str.m_end]   = '\0';
        }
        void setAttribute(str_t& str, int a) { return prefixAndNumberAndValue(str, a, 'm'); }

        
    }  // namespace nescape_codes
}  // namespace ncore