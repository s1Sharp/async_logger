#include <iostream>

#include "logger/logger.h"

using namespace std;

int main()
{
    std::boolalpha(std::cout);

    LOG_INFO("started");
    LOG_ERR("program is empty");

    std::cout << "logs was dumped into file=" << LOG_DUMP() << std::endl;
    std::cout << "logs was dumped into file=" << LOG_DUMP() << std::endl;

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
    LOG_INFO("Hello, World!");
    LOG_INFO("Hello, World!");
    LOG_INFO("Hello, World!");
    LOG_DEBUG("Sleep 10 seconds");
    std::this_thread::sleep_for(10s);
    try {
        throw std::exception("some error");
    } catch (std::exception) {
        LOG_INFO("Exception Catch");
    }
    int a = 5;
    LOG_DEBUG(std::format("Variable a={}", a));
    std::cout << "logs was dumped into file=" << LOG_DUMP() << std::endl;
	return 0;
}