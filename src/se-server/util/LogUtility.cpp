#include "LogUtility.h"

#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#endif

LogUtility* sLog = new LogUtility();

LogUtility::LogUtility() : m_logFile(NULL)
{
}

LogUtility::~LogUtility()
{
    if (m_logFile)
    {
        fclose(m_logFile);
        m_logFile = 0;
    }
}

void LogUtility::SetLogPath(const char* path)
{
    // Open Log File.
    m_logFile = fopen(path, "w");
}

void LogUtility::Debug(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteToLog("DBG", format, args);
    va_end(args);
}

void LogUtility::Info(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteToLog("INF", format, args);
    va_end(args);
}

void LogUtility::Warn(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteToLog("WRN", format, args);
    va_end(args);
}

void LogUtility::Error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteToLog("ERR", format, args);
    va_end(args);
}

void LogUtility::WriteToLog(const char* severity, const char* format, va_list args)
{
    char strFormat[1024] = {0};
    snprintf(strFormat, 1024, "[%s] %s\n", severity, format);
    vprintf(strFormat, args);

#ifdef WIN32
    char szBuffer[1024];
    int w = vsnprintf(szBuffer, sizeof(szBuffer), strFormat, args);

    if (w > 0)
    {
        OutputDebugString(szBuffer);

        if (m_logFile)
        {
            m_logFileMutex.lock();

            fwrite(szBuffer, 1, w, m_logFile);
            fflush(m_logFile);

            m_logFileMutex.unlock();
        }
    }
#endif
}

void LogUtility::HexDump(uint8_t* data, uint32_t size)
{
    uint32_t i, j;
    for (i = 0; i < size; i += 16)
    {
        printf("%06x: ", i);

        for (j = 0; j < 16; j++)
        {
            if (i + j < size)
                printf("%02x ", data[i + j]);
            else
                printf("   ");
        }

        printf(" ");

        for (j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                printf("%c", isprint(data[i + j]) ? data[i + j] : '.');
            }
        }
        printf("\n");
    }
}
