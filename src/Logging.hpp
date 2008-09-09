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

int Log(LogLevel, const char *, ...);
int StarryLog(LogLevel, unsigned starryness, const char *, ...);

#endif
