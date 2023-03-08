#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <thread>
#include <future>
#include <map>
#include <filesystem>

using DateTime = std::chrono::system_clock::time_point;

class Logger;

static Logger* m_instance = nullptr;
static std::condition_variable cv;
static std::atomic_bool quit = false;
static std::atomic_bool dump_log = false;
static std::mutex buf_m;
static std::stringstream ss_buf;
class Logger {
public:
    Logger(const Logger& root) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger* Instance()
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

    
    bool dumpLog() 
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

    void info(std::string&& message)
    {
        logMsgAdd(std::move(message), std::move(getChronoDateTime()), Logger::MsgLvl::INFO);
    };

    void debug(std::string&& message)
    {
        logMsgAdd(std::move(message), std::move(getChronoDateTime()), Logger::MsgLvl::DEBUG);
    };

    void error(std::string&& message)
    {
        logMsgAdd(std::move(message), std::move(getChronoDateTime()), Logger::MsgLvl::ERROR);
    };

private:
    enum class MsgLvl {
        INFO, DEBUG, ERROR
    };

    std::string msgLvlMapper(Logger::MsgLvl mlvl) 
    {
        static std::map<Logger::MsgLvl, std::string> msgLvlMapper = {
            {Logger::MsgLvl::INFO , "[INFO]  "},
            {Logger::MsgLvl::DEBUG, "[DEBUG] "},
            {Logger::MsgLvl::ERROR, "[ERROR] "},
        };
        return msgLvlMapper[mlvl];
    }

    inline DateTime getChronoDateTime()
    {
        return std::chrono::system_clock::now();
    }

    void logMsgAdd(std::string&& message, DateTime&& date, Logger::MsgLvl mlvl) 
    {
        const auto in_time_t = std::chrono::system_clock::to_time_t(date);
        {
            std::unique_lock<std::mutex> f_lk(buf_m);
            ss_buf
                << std::put_time(std::localtime(&in_time_t), "%d/%m/%y %X ")
                << msgLvlMapper(mlvl)
                << std::move(message)  // may be user already send '\n' symbol, --TODO add replacing
                << std::endl;
        }
    }

    static void writeIntoFile(const std::string& str)
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

    static void loggerWorker(void *)
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
                writeIntoFile(file_write_buf);
                dump_log = false;
            }
        }
    }

    Logger()
    {
        m_thread = new std::thread( Logger::loggerWorker, nullptr );
        m_thread->detach();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2s);
    };

    ~Logger()
    {
        quit = true;
        if (m_thread->joinable())
        {
            m_thread->join();
        }
        cv.notify_all();
    }
private:
    std::thread *m_thread;   
};


