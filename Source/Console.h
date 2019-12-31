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
    COLOR_PINK,
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

    //template<typename... ARGS>
    //void LogLine(const char* format, ARGS... args)
    void LogLine(const char* format, va_list args)
    {
        //va_list args;
        //va_copy(args, inputArgs);
        //va_start(args, format);

        char line[1024];
        char* str = line;

        int len = vsnprintf(line, sizeof(line), format, args);

        vector<char> buf;
        if (len >= sizeof(line))
        {
            buf.resize(len + 1);
            str = &buf[0];
    
            int written = vsnprintf(str, len, format, args);
            assert(written < len);
        }

        if (_logFile)
        {
            fprintf(_logFile, str);

            if (_debugOutput)
                fflush(_logFile);
        }

        WriteOutput(str);
        //va_end(args);
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
        _debugOutput = false;
        _logFile = NULL;

        // Disable output buffering

        std::cout.setf(std::ios::unitbuf);

        // Enable color by default if we're running in a console

#ifdef _MSC_VER
        DWORD processId = 0;
        GetWindowThreadProcessId(GetConsoleWindow(), &processId);
        if (processId == GetCurrentProcessId())
            _enableColor = true;
#else
        if (isatty(STDOUT_FILENO))
            _enableColor = true;
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

    //template<typename... ARGS>
    //void Print(const char* format, ARGS... args)
    void Print(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        LogLine(format, args);
        va_end(args);
    }

    //template<typename... TARGS>
    //void PrintColor(int color, const char* format, TARGS... args)
    void PrintColor(int color, const char* format, ...)
    {
        SetTextColor(color);

        va_list args;
        va_start(args, format);
        LogLine(format, args);
        va_end(args);

        ResetTextColor();
    }

    //template<typename... TARGS>
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

    string ReadLine()
    {
        string source;
        std::getline(std::cin, source);
        return source;
    }
};

