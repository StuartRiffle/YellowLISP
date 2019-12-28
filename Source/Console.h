// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#pragma once

enum
{
    ANSI_BLACK = 0,
    ANSI_RED,
    ANSI_GREEN,
    ANSI_YELLOW,
    ANSI_BLUE,
    ANSI_MAGENTA,
    ANSI_CYAN,
    ANSI_WHITE,

    ANSI_FOREGROUND = 30,
    ANSI_BACKGROUND = 40
};

inline WORD AnsiColorToWindows(int fg, int bg)
{
    WORD attr = 0;

    switch (fg)
    {
        case ANSI_RED:       attr |= FOREGROUND_RED; break;
        case ANSI_GREEN:     attr |= FOREGROUND_GREEN; break;
        case ANSI_YELLOW:    attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case ANSI_BLUE:      attr |= FOREGROUND_BLUE; break;
        case ANSI_MAGENTA:   attr |= FOREGROUND_RED | FOREGROUND_BLUE; break;
        case ANSI_CYAN:      attr |= FOREGROUND_RED | FOREGROUND_GREEN; break;
        case ANSI_WHITE:     attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
    }

    switch (bg)
    {
        case ANSI_RED:       attr |= BACKGROUND_RED; break;
        case ANSI_GREEN:     attr |= BACKGROUND_GREEN; break;
        case ANSI_YELLOW:    attr |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY; break;
        case ANSI_BLUE:      attr |= BACKGROUND_BLUE; break;
        case ANSI_MAGENTA:   attr |= BACKGROUND_RED | BACKGROUND_BLUE; break;
        case ANSI_CYAN:      attr |= BACKGROUND_RED | BACKGROUND_GREEN; break;
        case ANSI_WHITE:     attr |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY; break;
    }

    return attr;
}

extern bool gColorConsole;

inline void SetTextColor(int fg, int bg = 0)
{
    if (!gColorConsole)
        return;

#ifdef _MSC_VER
    WORD attr = AnsiColorToWindows(fg, bg);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr);
#else
    std::cout << "\u001b[" << ANSI_FOREGROUND + fg << "m";
    std::cout << "\u001b[" << ANSI_BACKGROUND + bg << "m";
#endif
}

inline void ResetTextColor()
{
    if (!gColorConsole)
        return;

#ifdef _MSC_VER
    WORD attr = AnsiColorToWindows(ANSI_WHITE, ANSI_BLACK);
    attr &= ~FOREGROUND_INTENSITY;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr);
#else
    std::cout << "\u001b[0m";
#endif
}

