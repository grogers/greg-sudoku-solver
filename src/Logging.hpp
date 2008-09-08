#ifndef LOGGING_HPP
#define LOGGING_HPP

enum LogLevel
{
    Fatal,
    Error,
    Warning,
    Info,
    Debug,
    Trace
};

LogLevel GetLogLevel();
void SetLogLevel(LogLevel);

const char *GetLogLevelName(LogLevel);

int Log(const char *, ...);

#endif
