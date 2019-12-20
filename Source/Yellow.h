// YellowLISP (c) 2019 Stuart Riffle

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define VERSION_MAJOR   (0)
#define VERSION_MINOR   (1)
#define VERSION_PATCH   (0)

#define RETURN_SUCCESS          (0)
#define RETURN_PARSING_ERROR    (-1)
#define RETURN_RUNTIME_ERROR    (-2)
#define RETURN_INTERNAL_ERROR   (-3)

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

#include <unordered_map>
using std::unordered_map;

#include <memory>
#include <cassert>
#include <iostream>
#include <sstream>
#include <exception>


// This crap should be somewhere else

#define COLOR_TITLE  "\x1b[30m\x1b[43m" // Black on yellow
#define COLOR_PROMPT "\x1b[33m"         // Yellow
#define COLOR_ERROR  "\x1b[37m"         // White
#define COLOR_RESET  "\x1b[0m"          // Default gray

