#include "json/json.h"
#include "logger/logger.h"

using namespace std;

int main()
{
    Logger::Instance()->info("started");
    Logger::Instance()->error("program is empty");
    std::cout << Logger::Instance()->dumpLog() << std::endl;
    std::cout << Logger::Instance()->dumpLog() << std::endl;

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
    std::cout << Logger::Instance()->dumpLog() << std::endl;
	return 0;
}