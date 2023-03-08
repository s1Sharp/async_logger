#include <filesystem>
#include <iostream>
#include <fstream>
#include <mutex>
#include <map>
#include <condition_variable>

#include "logger/logger.h"

// 1mb size of buffer
static constexpr size_t fixed_size_ss_buf = 1024 * 1024;

static Logger* m_instance = nullptr;

static std::atomic_bool quit = false;
static std::atomic_bool dump_log = false;

static std::condition_variable cv;

static std::mutex buf_m;
static std::stringstream ss_buf;

Logger* Logger::Instance()
{
    static bool m_instance_guard = false;
    static std::mutex m_mutex;
    if (!m_instance_guard) {
        if (!m_instance) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_instance_guard) {
                if (m_instance == nullptr) {
                    m_instance_guard = true;
                    m_instance = new Logger();
                }
            }
        }
    }
    return m_instance;
}

    
bool Logger::dumpLog() 
{
    static size_t last_dump_unix_time = 0;
    using namespace std::chrono;
    const size_t curr_unix_time =
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

    // prevent massive log dumping (this is design. We also can append information at the end of file, but ..)
    if (last_dump_unix_time < curr_unix_time) {
        last_dump_unix_time = curr_unix_time;
        dump_log = true;
        cv.notify_all();
        return true;
    }
    return false;
}

void Logger::info(std::string&& message)
{
    logMsgAdd(std::move(message), std::move(getChronoDateTime()), Logger::MsgLvl::INFO);
};

void Logger::debug(std::string&& message)
{
    logMsgAdd(std::move(message), std::move(getChronoDateTime()), Logger::MsgLvl::DEBUG);
};

void Logger::error(std::string&& message)
{
    logMsgAdd(std::move(message), std::move(getChronoDateTime()), Logger::MsgLvl::ERROR);
};

std::string Logger::msgLvlMapper(Logger::MsgLvl mlvl) 
{
    static std::map<Logger::MsgLvl, std::string> msgLvlMapper = {
        {Logger::MsgLvl::INFO , "[INFO]  "},
        {Logger::MsgLvl::DEBUG, "[DEBUG] "},
        {Logger::MsgLvl::ERROR, "[ERROR] "},
    };
    return msgLvlMapper[mlvl];
}

inline DateTime Logger::getChronoDateTime()
{
    return std::chrono::system_clock::now();
}

void Logger::logMsgAdd(std::string&& message, DateTime&& date, Logger::MsgLvl mlvl) 
{
    const auto in_time_t = std::chrono::system_clock::to_time_t(date);
    {
        std::unique_lock<std::mutex> f_lk(buf_m);
        ss_buf
            << std::put_time(std::localtime(&in_time_t), "%d/%m/%y %X ")
            << msgLvlMapper(mlvl)
            << std::move(message)  // may be user already send '\n' symbol, --TODO add replacing
            << std::endl;
        const size_t ss_buf_size = ss_buf.str().length();
        if (ss_buf_size > fixed_size_ss_buf) {
            // buff rotation (clear old logs data)
            const size_t new_start = ss_buf.str().find('\n', ss_buf_size - fixed_size_ss_buf);
            ss_buf.str(ss_buf.str().substr(new_start, ss_buf_size - new_start));
        }
    }
}

void Logger::writeIntoFile(const std::string& str)
{
    using namespace std::chrono;
    const size_t unix_time = 
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    
    const std::string filename = std::format("logs_{}.txt", unix_time);
    std::filesystem::path path{ std::filesystem::current_path() / "logs" };
    path /= filename;
    std::filesystem::create_directories(path.parent_path());

    std::ofstream ofs(path);
    ofs << str;
    ofs.close();
}

void Logger::loggerWorker(void *)
{
    std::mutex cv_m;
    while (!quit) {
        std::unique_lock<std::mutex> lk(cv_m);
        cv.wait(lk, []() { 
            return dump_log || quit;
        });

        if (dump_log) {
            std::string file_write_buf;
            { // get string from log buf with mutex
                std::unique_lock<std::mutex> f_lk(buf_m);
                file_write_buf = std::move(ss_buf.str());
                ss_buf.str("");
            } // after clear buffer unlock mutex
            // use string for dump into logfile.txt
            if (!file_write_buf.empty())
                // dump logs to file if log buf not empty
                writeIntoFile(file_write_buf);
            dump_log = false;
        }
    }
}

Logger::Logger()
{
    m_thread = new std::thread( Logger::loggerWorker, nullptr );
    m_thread->detach();
};

Logger::~Logger()
{
    quit = true;
    if (m_thread->joinable())
    {
        m_thread->join();
    }
    cv.notify_all();
}