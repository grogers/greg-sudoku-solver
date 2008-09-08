#include "Logging.hpp"
#include <cstdio>
#include <cstdarg>

namespace {
    LogLevel level = Fatal;
}

LogLevel GetLogLevel()
{
    return level;
}

void SetLogLevel(LogLevel newLevel)
{
    level = newLevel;
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

int Log(const char *fmt, ...)
{
    va_args va;

    va_start(va, fmt);
    vprintf(fmt, va);
    va_end(va);
}

