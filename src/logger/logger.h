#pragma once

#include <chrono>
#include <thread>

using DateTime = std::chrono::system_clock::time_point;

class Logger {
public:
    Logger(Logger&& root) = delete;
    Logger(const Logger& root) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger* Instance();

    bool dumpLog();

    void info(std::string&& message);
    void debug(std::string&& message);
    void error(std::string&& message);

private:
    enum class MsgLvl {
        INFO, DEBUG, ERROR
    };

    std::string msgLvlMapper(Logger::MsgLvl mlvl);

    inline DateTime getChronoDateTime();
    void logMsgAdd(std::string&& message, DateTime&& date, Logger::MsgLvl mlvl);
    static void writeIntoFile(const std::string& str);
    static void loggerWorker(void *);

    Logger();
    ~Logger();

private:
    std::thread *m_thread;
    bool func_line_log_format;
};

// return bool
#define LOG_DUMP() Logger::Instance()->dumpLog(msg)

#define LOG_SET_FUNC_LINE_FORMAT

#ifdef LOG_SET_FUNC_LINE_FORMAT
    #if defined(_WIN32) || defined(__CYGWIN__)
        #define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
    #elif (defined(__APPLE__) && defined(__MACH__)) || defined(unix) || defined(__unix__) || defined(__unix) || defined(__linux__)
        #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
    #else
        #error Unknown environment!
    #endif

    #define ADDITIONAL_DEBUG_INFO std::format(" ({}:{}:{})", std::string(__FILENAME__),  std::string(__func__), std::to_string(__LINE__))
    #define LOG_INFO(msg) Logger::Instance()->info(msg + ADDITIONAL_DEBUG_INFO)
    #define LOG_DEBUG(msg) Logger::Instance()->debug(msg + ADDITIONAL_DEBUG_INFO)
    #define LOG_ERR(msg) Logger::Instance()->error(msg + ADDITIONAL_DEBUG_INFO)
#elif
    #define LOG_INFO(msg) Logger::Instance()->info(msg)
    #define LOG_DEBUG(msg) Logger::Instance()->debug(msg)
    #define LOG_ERR(msg) Logger::Instance()->error(msg)
#endif
