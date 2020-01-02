// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once

enum
{
    COLOR_BLACK = 0,
    COLOR_NAVY,
    COLOR_GREEN,
    COLOR_TEAL,
    COLOR_RED,
    COLOR_PURPLE,
    COLOR_BROWN,
    COLOR_SILVER,

    COLOR_GRAY,
    COLOR_BLUE,
    COLOR_LIME,
    COLOR_CYAN,
    COLOR_BRIGHT_RED,
    COLOR_MAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE,

    ANSI_FOREGROUND = 30,
    ANSI_BACKGROUND = 40,
    ANSI_BRIGHT     = 60
};

class Console
{
    bool _enableColor;
    bool _extendedCharSet;
    bool _debugOutput;
    FILE* _logFile;

    WORD ColorToWindowsAttrib(int fg, int bg)
    {
        int attr = (bg << 4) | fg;
        return (WORD) attr;
    }

    string ColorToAnsiEscapeCode(int fg, int bg)
    {
        int fg_color  = fg & 7;
        int fg_bright = fg & 8;
        int bg_color  = bg & 7;
        int bg_bright = bg & 8;

        const int mapping[] = { 0, 4, 2, 6, 1, 5, 3, 7 };

        fg = ANSI_FOREGROUND + mapping[fg_color] + (fg_bright? ANSI_BRIGHT : 0);
        bg = ANSI_BACKGROUND + mapping[bg_color] + (bg_bright? ANSI_BRIGHT : 0);

        std::stringstream ss;
        ss <<  "\u001b[" << fg << "m";
        ss <<  "\u001b[" << bg << "m";

        return ss.str();
    }

    void LogLine(const char* format, va_list args)
    {
        char line[128];
        char* str = line;

        int len = vsnprintf(line, sizeof(line), format, args);

        vector<char> buf;
        if (len >= sizeof(line))
        {
            buf.resize(len + 1);
            str = &buf[0];
    
            int written = vsnprintf(str, len, format, args);
            (written);

            assert(written <= len);
        }

        if (_logFile)
        {
            fprintf(_logFile, str);

            if (_debugOutput)
                fflush(_logFile);
        }

        WriteOutput(str);
    }

protected:
    virtual void WriteOutput(const char* str)
    {
        // Override this function to redirect output

        std::cout << str;
    }

public:
    Console()
    {
        _enableColor = false;
        _extendedCharSet = false;
        _debugOutput = false;
        _logFile = NULL;

        // Disable output buffering

        std::cout.setf(std::ios::unitbuf);

        // Detect color and extended character set

    #ifdef _MSC_VER
        _enableColor = true;

        HANDLE stdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdOutput != INVALID_HANDLE_VALUE)
        {
            DWORD mode = 0;
            if (GetConsoleMode(stdOutput, &mode))
            {
                mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                if (SetConsoleMode(stdOutput, mode))
                    _extendedCharSet = true;
            }
        }
    #else
        if (isatty(STDOUT_FILENO))
            _enableColor = true;

        if(strcmp(nl_langinfo(CODESET), "UTF-8") == 0)
            _extendedCharSet = true;
    #endif
    }

    void SetTextColor(int fg, int bg = COLOR_BLACK)
    {
        if (!_enableColor)
            return;

#ifdef _MSC_VER
        WORD attr = ColorToWindowsAttrib(fg, bg);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr);
#else
        Print(ColorToAnsiEscapeCode(fg, bg));
#endif
    }

    inline void ResetTextColor()
    {
        if (!_enableColor)
            return;

#ifdef _MSC_VER
        WORD attr = ColorToWindowsAttrib(COLOR_SILVER, COLOR_BLACK);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr);
#else
        Print("\u001b[0m");
#endif
    }

    void EnableColor(bool enabled)
    {
        _enableColor = enabled;
    }

    bool IsColor()
    {
        return _enableColor;
    }

    bool IsExtendedCharSet()
    {
        return _extendedCharSet;
    }

    void EnableDebugOutput(bool enabled)
    {
        _debugOutput = enabled;
    }

    void LogToFile(const char* filename = NULL)
    {
        if (_logFile)
            fclose(_logFile);

        if (filename)
            _logFile = fopen(filename, "a");
    }

    void Print(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        LogLine(format, args);
        va_end(args);
    }

    void PrintColor(int fg, int bg, const char* format, ...)
    {
        SetTextColor(fg, bg);

        va_list args;
        va_start(args, format);
        LogLine(format, args);
        va_end(args);

        ResetTextColor();
    }

    void PrintDebug(const char* format, ...)
    {
        if (!_debugOutput)
            return;

        SetTextColor(COLOR_GRAY);

        va_list args;
        va_start(args, format);
        LogLine(format, args);
        va_end(args);

        ResetTextColor();
    }

    void PrintErrorPrefix(const char* desc = "ERROR", int code = 0)
    {
        if (_enableColor)
        {
            PrintColor(COLOR_WHITE, COLOR_RED, " %s ", desc);
            if (code)
                PrintColor(COLOR_WHITE, COLOR_RED, "%d ", code);
        }
        else
        {
            Print(desc);
            if (code)
                Print(" %d", code);
            Print(":");
        }

        Print(" ");
    }

    void PrintColorTest()
    {
        for (int fg = 0; fg < 16; fg++)
        {
            for (int bg = 0; bg < 16; bg++)
            {
                SetTextColor(fg, bg);
                std::cout << "(a)";
            }
            ResetTextColor();
            std::cout << std::endl;
        }
    }

    string ReadLine()
    {
        string source;
        std::getline(std::cin, source);
        return source;
    }
};

