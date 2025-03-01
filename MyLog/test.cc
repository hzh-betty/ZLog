#include "util.hpp"
#include "level.hpp"
#include "message.hpp"
#include "format.hpp"
#include "sink.hpp"
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
    mylog::Formmatter fmt("abc%%abc[%d{%H:%M:%S]} %T%m%n");
    std::string str = fmt.format(logmsg);
    // mylog::LogSink::ptr stdout_ptr = mylog::SinkFactory::create<mylog::StdOutSink>();
    // mylog::LogSink::ptr file_ptr = mylog::SinkFactory::create<mylog::FileSink>("./logfile/test.log");
    // mylog::LogSink::ptr roll_ptr = mylog::SinkFactory::create<mylog::RollBySizeSink>("./logfile/roll-", 1024 * 1024);
    // stdout_ptr->log(str.c_str(), str.size());
    // file_ptr->log(str.c_str(), str.size());
    // roll_ptr->log(str.c_str(), str.size());

    // size_t curSize = 0;
    // size_t cnt = 0;
    // while (curSize < 1024 * 1024 * 10)
    // {
    //     std::string tmp = str + std::to_string(cnt++);
    //     roll_ptr->log(tmp.c_str(), tmp.size());
    //     curSize += tmp.size();
    // }

    mylog::LogSink::ptr time_ptr = mylog::SinkFactory::create<mylog::RollByTimeSink>("./logfile/roll-", mylog::TimeGap::GAP_SECOND);
    time_t cur = mylog::Date::getCurrentTime();
    while (mylog::Date::getCurrentTime() < cur + 5)
    {
        time_ptr->log(str.c_str(), str.size());
    }
    return 0;
}