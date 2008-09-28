#ifndef LOGGING_HPP
#define LOGGING_HPP

enum LogLevel
{
    Never,
    Fatal,
    Error,
    Warning,
    Info,
    Debug,
    Trace
};

void SetLogLevel(LogLevel);
LogLevel QuietlyBifurcate();

void SetShouldPrintLogLevel(bool);
void SetShouldQuietlyBifurcate(bool);

const char *GetLogLevelName(LogLevel);

int Log(LogLevel, const char *, ...);

#endif
