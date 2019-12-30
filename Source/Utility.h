// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"

string Uppercase(const string& str)
{
    string result = str;

    for( size_t i = 0; i < result.length(); i++)
        result[i] = (char) toupper(result[i]);

    return result;
}
