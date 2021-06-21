#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>

class Logger {};

inline void logd(std::string format, ...) {
    format += '\n';
    va_list va;
    va_start(va, format);
    vprintf(format.c_str(), va);
    va_end(va);
}