#include "Logging.hpp"
#include <cstdio>
#include <cstdarg>

namespace {
    LogLevel level = Info;
    bool shouldPrintLogLevel = false;
}

void SetLogLevel(LogLevel newLevel)
{
    level = newLevel;
}

void SetShouldPrintLogLevel(bool x)
{
    shouldPrintLogLevel = x;
}

const char *GetLogLevelName(LogLevel level)
{
    switch (level)
    {
        case Fatal: return "Fatal";
        case Error: return "Error";
        case Warning: return "Warning";
        case Info: return "Info";
        case Debug: return "Debug";
        case Trace: return "Trace";
        default: return "Unknown";
    }
}

int Log(LogLevel lvl, const char *fmt, ...)
{
    if (level < lvl)
        return 0;

    va_list va;

    va_start(va, fmt);
    if (shouldPrintLogLevel)
        printf("%s: ", GetLogLevelName(lvl));
    int ret = vprintf(fmt, va);
    va_end(va);

    return ret;
}

int StarryLog(LogLevel lvl, unsigned numStarLines, const char *fmt, ...)
{
    if (level < lvl)
        return 0;

    va_list va;

    for (unsigned i = 0; i < numStarLines; ++i) {
        for (unsigned j = 0; j <= i; ++j)
            printf("*");
        printf("\n");
    }

    va_start(va, fmt);
    if (shouldPrintLogLevel)
        printf("%s: ", GetLogLevelName(lvl));
    int ret = vprintf(fmt, va);
    va_end(va);

    for (unsigned i = numStarLines; i > 0; --i) {
        for (unsigned j = i; j > 0; --j)
            printf("*");
        printf("\n");
    }

    return ret;
}
