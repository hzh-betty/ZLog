#include "../mylog/log.h"

/*
    %d 表示日期--包含子格式{%H-%M-%S}
    %t 线程ID
    %c 日志器名称
    %f 源码文件名
    %l 行号
    %p 日志级别
    %T 制表符缩进
    %m 主体消息
    %n 表示换行
*/

// 通过自定义日志器打印
void loggerTest1()
{
    // 1. 创建全局日志器建造器,可以在全局使用
    std::unique_ptr<mylog::GlobalLoggerBuilder> builder(new mylog::GlobalLoggerBuilder());

    // 2. 输入参数，必须要日志器名称!!!
    builder->buildLoggerName("async_logger");
    builder->buildLoggerLevel(mylog::LogLevel::value::WARNING);                          // 最低输出日志等级，默认为DEBUG
    builder->buildLoggerFormmater("[%c][%f:%l]%T%m%n");                                  // 日志输出格式
    builder->buildLoggerType(mylog::LoggerType::LOGGER_ASYNC);                           // 启动异步输出
    builder->buildEnalleUnSafe();                                                        // 测试时启动非安全模式，其他情况不要启动安全模式！
    builder->buildLoggerSink<mylog::FileSink>("./logfile/async.log");                    // 文件
    builder->buildLoggerSink<mylog::StdOutSink>();                                       // 显示屏
    builder->buildLoggerSink<mylog::RollBySizeSink>("./logfile/roll-", 1024 * 1024 * 5); // 滚动文件

    // 3. 获取对应的日志器
    mylog::Logger::ptr logger = builder->build();

    // 4. 测试
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

// DEBUG INFO WARN ERROR FATAL 宏可以通过直接调用默认全局日志器使用 --(DEBUG，LOGGER_SYNC, 安全)
void loggerTest2()
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


//  滚动文件(以时间)
enum class TimeGap
{
    GAP_SECOND,
    GAP_MINUTE,
    GAP_HOUR,
    GAP_DAY,
};

class RollByTimeSink : public mylog::LogSink
{
public:
    RollByTimeSink(const std::string &basename, TimeGap gaptype)
        : basename_(basename)
    {
        switch (gaptype)
        {
        case TimeGap::GAP_SECOND:
            gapSize_ = 1;
            break;
        case TimeGap::GAP_MINUTE:
            gapSize_ = 60;
            break;
        case TimeGap::GAP_HOUR:
            gapSize_ = 3600;
            break;
        case TimeGap::GAP_DAY:
            gapSize_ = 3600 * 24;
            break;
        }

        // 1.获取当前时间节点
        curGap_ = mylog::Date::getCurrentTime() / gapSize_;
        std::string filename = createNewFile();
        mylog::File::createDiretory(mylog::File::path(filename));
        ofs_.open(filename, std::ios::binary | std::ios::app);
        assert(ofs_.is_open());
    }

    void log(const char *log, size_t len) override
    {
        // 如果时间节点不同则创建新文件
        time_t cur = mylog::Date::getCurrentTime();
        if (cur / gapSize_ != curGap_)
        {
            ofs_.close();
            std::string pathname = createNewFile();
            ofs_.open(pathname, std::ios::binary | std::ios::app);
        }
        ofs_.write(log, len);
        assert(ofs_.good());
    }

protected:
    // 以时间为文件名
    std::string createNewFile()
    {
        time_t t = mylog::Date::getCurrentTime();
        struct tm lt;
        localtime_r(&t, &lt);
        std::stringstream filename;
        filename << basename_ << (lt.tm_year + 1900) << (lt.tm_mon + 1) << lt.tm_mday
                 << lt.tm_hour << lt.tm_min << lt.tm_sec << ".log";
        return filename.str();
    }

protected:
    std::string basename_;
    std::ofstream ofs_;
    size_t curGap_;  // 当前是第几个时间段
    size_t gapSize_; // 时间段的大小
};

// 支持落地方式扩展，列如滚动时间
void loggerTest3()
{
    // 1. 创建全局日志器建造器,可以在全局使用
    std::unique_ptr<mylog::GlobalLoggerBuilder> builder(new mylog::GlobalLoggerBuilder());

    // 2. 输入参数，必须要日志器名称!!!
    builder->buildLoggerName("async_logger");
    builder->buildLoggerLevel(mylog::LogLevel::value::WARNING); // 最低输出日志等级，默认为DEBUG
    builder->buildLoggerFormmater("[%c][%f:%l]%T%m%n");         // 日志输出格式
    builder->buildLoggerType(mylog::LoggerType::LOGGER_ASYNC);  // 启动异步输出
    builder->buildEnalleUnSafe();                               // 测试时启动非安全模式，其他情况不要启动安全模式！
    builder->buildLoggerSink<RollByTimeSink>("./logfile/time-", TimeGap::GAP_SECOND); // 滚动文件
    mylog::Logger::ptr logger = builder->build();
    time_t cur = mylog::Date::getCurrentTime();
    size_t cnt = 0;
    while (mylog::Date::getCurrentTime() < cur + 5)
    {
        logger->fatal("%s%d", "测试日志-", cnt++);
    }
}
int main(int argc, char *argv[])
{
    // loggerTest1();
    //loggerTest2();
    loggerTest3();
    return 0;
}