#include "util.hpp"
#include "level.hpp"
#include "message.hpp"
#include "format.hpp"
#include <thread>

int main()
{
    // std::cout<<MyLog::Date::getCurrentTime()<<std::endl;
    // std::string pathname = "./abc/def/ghi";
    // MyLog::File::createDiretory(MyLog::File::path(pathname));

    // std::cout << MyLog::LogLevel::toString(MyLog::LogLevel::value::DEBUG) << std::endl;
    // std::cout << MyLog::LogLevel::toString(MyLog::LogLevel::value::INFO) << std::endl;
    // std::cout << MyLog::LogLevel::toString(MyLog::LogLevel::value::WARNING) << std::endl;
    // std::cout << MyLog::LogLevel::toString(MyLog::LogLevel::value::ERROR) << std::endl;
    // std::cout << MyLog::LogLevel::toString(MyLog::LogLevel::value::OFF) << std::endl;
    // std::cout << MyLog::LogLevel::toString(MyLog::LogLevel::value::FATAL) << std::endl;

    std::string file = "test.cc";
    mylog::LogMessage logmsg(mylog::LogLevel::value::INFO, file, 19, "这是一个测试", "master");
    mylog::Formmatter fmt("abc%%abc[%d{%H-%M-%S]} %e%m%n");
    std::cout << fmt.format(logmsg);
    return 0;
}