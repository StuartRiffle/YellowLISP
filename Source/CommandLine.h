// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"

class CommandLine
{
    int _argc;
    char** _argv;

public:
    CommandLine(int argc, char** argv) : _argc(argc), _argv(argv) {}

    bool HasFlag(const char* name)
    {
        for (int i = 1; i < _argc; i++)
            if (!strcmp(_argv[i], name))
                return true;

        return false;
    }

    vector<string> ArgsEndingWith(const char* ext)
    {
        vector<string> result;
        size_t extLength = strlen(ext);

        for (int i = 1; i < _argc; i++)
        {
            size_t argLength = strlen(_argv[i]);
            if (argLength >= extLength)
                if (!strcmp(_argv[i] + argLength - extLength, ext))
                    result.push_back(_argv[i]);
        }

        return result;
    }
};
