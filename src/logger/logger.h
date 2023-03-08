#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <thread>
#include <future>
#include <map>

using DateTime = std::chrono::system_clock::time_point;

using args = std::tuple<
        std::string,
        DateTime
        >;


std::condition_variable cv;
std::mutex cv_m;
std::atomic_bool quit;


class Logger;

static Logger* m_instance = nullptr;

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

    enum class MsgLvl {
        INFO, DEBUG, ERROR
    };



    std::string msgLvlMapper(Logger::MsgLvl mlvl) 
    {
        static std::map<Logger::MsgLvl, std::string> msgLvlMapper = {
            {Logger::MsgLvl::INFO, "INFO"},
            {Logger::MsgLvl::DEBUG, "DEBUG"},
            {Logger::MsgLvl::ERROR, "ERROR"},
        };
        return msgLvlMapper[mlvl];
    }

    void logMsgAdd(const std::string& message) 
    {

    }

    void info(const std::string& message)
    {
        os << "   info: " << message << std::endl;
    };

    void debug(const std::string& message)
    {
        os << "   info: " << message << std::endl;
    };

    void error(const std::string& message)
    {
        os << "warning: " << message << std::endl;
    };

private:
    std::ofstream           m_File;

    Logger():os{std::cerr} {

    };

    std::queue<args> msgs;
    std::ostream& os;
};
