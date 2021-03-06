// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#ifndef __YELLOWLISP_H
#define __YELLOWLISP_H

#ifdef _MSC_VER
    #define _CRT_SECURE_NO_WARNINGS
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#else
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>
    #include <langinfo.h>
    #include <math.h>
#endif

#ifndef NDEBUG
#define DEBUG_BUILD (1)
#endif

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x)      STRINGIFY_IMPL(x)

#define RELEASE_BUILD (!DEBUG_BUILD )

#define VERSION_MAJOR           (0)
#define VERSION_MINOR           (1)
#define VERSION_PATCH           (0)

#define RETURN_SUCCESS          (0)
#define RETURN_PARSING_ERROR    (-1)
#define RETURN_RUNTIME_ERROR    (-2)
#define RETURN_INTERNAL_ERROR   (-3)

#ifndef YELLOW_THREAD_SAFE
#define YELLOW_THREAD_SAFE (1)
#endif

#ifndef YELLOW_CATCH_EXCEPTIONS
#define YELLOW_CATCH_EXCEPTIONS (1)
#endif

#ifndef YELLOW_ENABLE_GC
#define YELLOW_ENABLE_GC (1)
#endif

#include <cstdint>
using std::uint32_t;
using std::uint64_t;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <list>
using std::list;

#include <map>
using std::map;

#include <set>
using std::set;

#include <unordered_map>
using std::unordered_map;

#include <mutex>
#include <memory>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <algorithm>
#include <cstdarg>

#endif // __YELLOWLISP_H
