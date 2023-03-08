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

class Logger;

static Logger* m_instance = nullptr;
static std::condition_variable cv;

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

    using args = std::tuple<
        std::string,
        DateTime,
        Logger::MsgLvl
        >;


    std::string msgLvlMapper(Logger::MsgLvl mlvl) 
    {
        static std::map<Logger::MsgLvl, std::string> msgLvlMapper = {
            {Logger::MsgLvl::INFO , "INFO "},
            {Logger::MsgLvl::DEBUG, "DEBUG"},
            {Logger::MsgLvl::ERROR, "ERROR"},
        };
        return msgLvlMapper[mlvl];
    }

    void logMsgAdd(std::string&& message, DateTime&& date, Logger::MsgLvl mlvl) 
    {
        msgs.emplace(std::move(message), std::move(date), mlvl);
        cv.notify_all();
    }

    void info(std::string&& message)
    {
        os << "   info: " << message << std::endl;
        auto in_time_t = std::chrono::system_clock::to_time_t(getChronoDateTime());

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%d/%m/%y %X");

        os << "   info: " << message <<ss.str() << std::endl;
        logMsgAdd(std::move(message), std::move(getChronoDateTime()), Logger::MsgLvl::INFO);

    };

    void debug(std::string&& message)
    {
        logMsgAdd(std::move(message), std::move(getChronoDateTime()), Logger::MsgLvl::DEBUG);
        os << "   info: " << message << std::endl;
    };

    void error(std::string&& message)
    {
        logMsgAdd(std::move(message), std::move(getChronoDateTime()), Logger::MsgLvl::ERROR);
        os << "warning: " << message << std::endl;
    };

private:
    inline DateTime getChronoDateTime()
    {
        return std::chrono::system_clock::now();
    }

    static void loggerWorker(std::queue<args>& q)
    {
        std::mutex cv_m;
        std::atomic_bool quit = false;
        
        while (!quit) {
            std::unique_lock<std::mutex> lk(cv_m);
            //std::cerr << std::this_thread::get_id() << " waiting... " << std::endl;
            cv.wait(lk, [&q, &quit]() { return !q.empty() || quit; });

            if (!q.empty()) {
                auto [msg, date, lvl] = std::move(q.front());
                q.pop();
                auto s = q.size();
                lk.unlock();
                std::cerr << std::this_thread::get_id() << " pop=" << msg << " size=" << s << std::endl;
            }
        }
    }

private:
    std::ofstream           m_File;

    Logger():os{std::cerr} {
        std::thread t1(Logger::loggerWorker, std::ref(msgs));
        t1.detach();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2s);
    };
    std::queue<args> msgs;
    std::ostream& os;
};


