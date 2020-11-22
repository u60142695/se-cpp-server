#ifndef LOG_UTILITY_H
#define LOG_UTILITY_H

#include <iostream>
#include <mutex>
#include <stdarg.h>
#include <stdint.h>

class LogUtility
{
   public:
    LogUtility();
    ~LogUtility();

    void SetLogPath(const char* path);

    void Debug(const char* format, ...);
    void Info(const char* format, ...);
    void Warn(const char* format, ...);
    void Error(const char* format, ...);

    void HexDump(uint8_t* data, uint32_t size);

   private:
    void WriteToLog(const char* severity, const char* format, va_list args);

   private:
    FILE* m_logFile;
    std::mutex m_logFileMutex;
};

extern LogUtility* sLog;

#endif
