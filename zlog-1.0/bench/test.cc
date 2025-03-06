#include "../zlog/zlog.h"
#include <thread>
#include <fstream>

/* 一些写代码过程中设计的单元测试 */

// 测试公共功能接口的设计
void test1()
{
    std::cout << zlog::Date::getCurrentTime() << std::endl;
    std::string pathname = "./abc/def/ghi";
    zlog::File::createDiretory(zlog::File::path(pathname));
}
// 测试日志等级
void test2()
{
    std::cout << zlog::LogLevel::toString(zlog::LogLevel::value::DEBUG) << std::endl;
    std::cout << zlog::LogLevel::toString(zlog::LogLevel::value::INFO) << std::endl;
    std::cout << zlog::LogLevel::toString(zlog::LogLevel::value::WARNING) << std::endl;
    std::cout << zlog::LogLevel::toString(zlog::LogLevel::value::ERROR) << std::endl;
    std::cout << zlog::LogLevel::toString(zlog::LogLevel::value::OFF) << std::endl;
    std::cout << zlog::LogLevel::toString(zlog::LogLevel::value::FATAL) << std::endl;
}

// 测试日志格式化
void test3()
{
    std::string file = "test.cc";
    zlog::LogMessage logmsg(zlog::LogLevel::value::INFO, file, 19, "这是一个测试", "master");
    zlog::Formmatter fmt("abc%%abc[%d{%H:%M:%S]} %T%m%n");
    std::string str = fmt.format(logmsg);
    std::cout << str << std::endl;
}

// 测试日志落地板块
void test4()
{
    std::string file = "test.cc";
    zlog::LogMessage logmsg(zlog::LogLevel::value::INFO, file, 19, "这是一个测试", "master");
    zlog::Formmatter fmt("abc%%abc[%d{%H:%M:%S]} %T%m%n");
    std::string str = fmt.format(logmsg);
    zlog::LogSink::ptr stdout_ptr = zlog::SinkFactory::create<zlog::StdOutSink>();
    zlog::LogSink::ptr file_ptr = zlog::SinkFactory::create<zlog::FileSink>("./logfile/test.log");
    zlog::LogSink::ptr roll_ptr = zlog::SinkFactory::create<zlog::RollBySizeSink>("./logfile/roll-", 1024 * 1024);
    stdout_ptr->log(str.c_str(), str.size());
    file_ptr->log(str.c_str(), str.size());
    roll_ptr->log(str.c_str(), str.size());

    size_t curSize = 0;
    size_t cnt = 0;
    while (curSize < 1024 * 1024 * 10)
    {
        std::string tmp = str + std::to_string(cnt++);
        roll_ptr->log(tmp.c_str(), tmp.size());
        curSize += tmp.size();
    }

}

// 测试同步日志器模块
void test5()
{

    std::string loggerName = "sync_logger";
    zlog::LogLevel::value limmit = zlog::LogLevel::value::WARNING;
    zlog::Formmatter::ptr formmatter(new zlog::Formmatter());
    zlog::LogSink::ptr stdout_ptr = zlog::SinkFactory::create<zlog::StdOutSink>();
    zlog::LogSink::ptr file_ptr = zlog::SinkFactory::create<zlog::FileSink>("./logfile/test.log");
    zlog::LogSink::ptr roll_ptr = zlog::SinkFactory::create<zlog::RollBySizeSink>("./logfile/roll-", 1024 * 1024);
    std::vector<zlog::LogSink::ptr> sinks = {stdout_ptr, file_ptr, roll_ptr};
    zlog::Logger::ptr logger(new zlog::SyncLogger(loggerName, limmit, formmatter, sinks));
    logger->debug(__FILE__, __LINE__, "%s", "测试日志");
    logger->info(__FILE__, __LINE__, "%s", "测试日志");
    logger->warn(__FILE__, __LINE__, "%s", "测试日志");
    logger->error(__FILE__, __LINE__, "%s", "测试日志");

    size_t curSize = 0;
    size_t cnt = 0;
    while (curSize < 1024 * 1024 * 10)
    {
        logger->fatal(__FILE__, __LINE__, "%s%d", "测试日志-", cnt++);
        curSize += 20;
    }
}

// 测试建造者模式
void test6()
{
    std::unique_ptr<zlog::LocalLoggerBuilder> builder(new zlog::LocalLoggerBuilder());
    builder->buildLoggerName("sync_logger");
    builder->buildLoggerLevel(zlog::LogLevel::value::WARNING);
    builder->buildLoggerFormmater("%m%n");
    builder->buildLoggerType(zlog::LoggerType::LOGGER_SYNC);
    builder->buildLoggerSink<zlog::FileSink>("./logfile/test.log");
    builder->buildLoggerSink<zlog::StdOutSink>();
    zlog::Logger::ptr logger = builder->build();
    logger->debug(__FILE__, __LINE__, "%s", "测试日志");
    logger->info(__FILE__, __LINE__, "%s", "测试日志");
    logger->warn(__FILE__, __LINE__, "%s", "测试日志");
    logger->error(__FILE__, __LINE__, "%s", "测试日志");
    logger->fatal(__FILE__, __LINE__, "%s", "测试日志");

    size_t curSize = 0;
    size_t cnt = 0;
    while (curSize < 1024 * 1024 * 10)
    {
        logger->fatal(__FILE__, __LINE__, "%s%d", "测试日志-", cnt++);
        curSize += 20;
    }
}

// 测试向缓冲区的写入与读取
void test7()
{
    std::ifstream ifs("./logfile/test.log", std::ios::binary);
    if (!ifs.is_open())
        return;
    ifs.seekg(0, std::ios::end);
    size_t fsize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::string body;
    body.resize(fsize);
    ifs.read(&body[0], fsize);
    if (!ifs.good())
    {
        std::cerr << "read error" << std::endl;
        return;
    }
    ifs.close();
    zlog::Buffer buffer;
    for (int i = 0; i < body.size(); i++)
    {
        buffer.push(&body[i], 1);
    }
    std::ofstream ofs("./logfile/tmp.log", std::ios::binary);
    size_t n = buffer.readAbleSize();
    for (int i = 0; i < n; i++)
    {
        ofs.write(buffer.begin(), 1);
        buffer.moveReader(1);
    }
    ofs.close();
}

// 测试异步日志器
void test8()
{
    std::unique_ptr<zlog::LocalLoggerBuilder> builder(new zlog::LocalLoggerBuilder());
    builder->buildLoggerName("async_logger");
    builder->buildLoggerLevel(zlog::LogLevel::value::WARNING);
    builder->buildLoggerFormmater("[%c][%f:%l]%T%m%n");
    builder->buildLoggerType(zlog::LoggerType::LOGGER_ASYNC);
    builder->buildEnalleUnSafe();
    builder->buildLoggerSink<zlog::FileSink>("./logfile/async.log");
    builder->buildLoggerSink<zlog::StdOutSink>();
    zlog::Logger::ptr logger = builder->build();
    logger->debug("%s", "测试日志");
    logger->info("%s", "测试日志");
    logger->warn("%s", "测试日志");
    logger->error("%s", "测试日志");
    logger->fatal("%s", "测试日志");

    size_t cnt = 0;
    while (cnt < 500000)
    {
        logger->fatal("%s%d", "测试日志-", cnt++);
    }
}

// 测试全局日志器建造者
void test9()
{
    zlog::Logger::ptr logger = zlog::LoggerManager::getInstance().getLogger("async_logger");
    logger->debug("%s", "测试日志");
    logger->info("%s", "测试日志");
    logger->warn("%s", "测试日志");
    logger->error("%s", "测试日志");
    logger->fatal("%s", "测试日志");

    size_t cnt = 0;
    while (cnt < 500000)
    {
        logger->fatal("%s%d", "测试日志-", cnt++);
    }
}

// 测试宏的替换
void test10()
{
    DEBUG("%s", "测试日志");
    INFO("%s", "测试日志");
    WARN("%s", "测试日志");
    ERROR("%s", "测试日志");
    FATAL("%s", "测试日志");

    size_t cnt = 0;
    while (cnt < 500000)
    {
        FATAL("%s%d", "测试日志-", cnt++);
    }
}
int main()
{
    // test1();
    // test2();
    // test3();
    // test4();
    // test5();
    // test6();
    // test7();
    // test8();
    // std::unique_ptr<zlog::GlobalLoggerBuilder> builder(new zlog::GlobalLoggerBuilder());
    // builder->buildLoggerName("async_logger");
    // builder->buildLoggerLevel(zlog::LogLevel::value::WARNING);
    // builder->buildLoggerFormmater("[%c][%f:%l]%T%m%n");
    // builder->buildLoggerType(zlog::LoggerType::LOGGER_ASYNC);
    // builder->buildEnalleUnSafe();
    // builder->buildLoggerSink<zlog::FileSink>("./logfile/async.log");
    // builder->buildLoggerSink<zlog::StdOutSink>();
    // zlog::Logger::ptr logger = builder->build();
    // test9();
    test10();
    return 0;
}