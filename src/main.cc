#include "json/json.h"
#include "logger/logger.h"

using namespace std;

int main()
{
	Json::Value root;   // starts as "null"; will contain the root value after parsing

	root["encoding"] = "UTF-8";
	root["indent"]["length"] = "Some text";
	root["indent"]["use_space"] = "Some user_space text";

	std::cout << root << "\n";
	/*auto ex1 = Json::parse(R"(
	  {
		"pi": 3.141,
		"happy": true
	  }
	)");*/
    Logger::Instance()->info("started");
    Logger::Instance()->error("program is empty");
	return 0;
}