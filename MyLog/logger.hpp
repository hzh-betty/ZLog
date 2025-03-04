/*
    完成日志器模块
        1. 同步日志器
        2. 异步日志器
*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "util.hpp"
#include "level.hpp"
#include "format.hpp"
#include "message.hpp"
#include "sink.hpp"
#include "looper.hpp"
#include <mutex>
#include <atomic>
#include <cstdarg>
#include <cstdio>

namespace mylog
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &loggerName, LogLevel::value limitLevel,
               Formmatter::ptr &formmatter,
               std::vector<LogSink::ptr> &sinks) : loggerName_(loggerName),
                                                   limitLevel_(limitLevel), formmatter_(formmatter), sinks_(sinks.begin(), sinks.end())
        {
        }

        void debug(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            // 1.判断消息级别
            if (LogLevel::value::DEBUG < limitLevel_)
                return;

            // 2. 提取不定参数
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed";
                va_end(ap);
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::DEBUG, file, line, res);
            free(res);
        }

        void info(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            // 1.判断消息级别
            if (LogLevel::value::INFO < limitLevel_)
                return;

            // 2. 提取不定参数
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed";
                va_end(ap);
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::INFO, file, line, res);
            free(res);
        }

        void warn(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            // 1.判断消息级别
            if (LogLevel::value::WARNING < limitLevel_)
                return;

            // 2. 提取不定参数
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed";
                va_end(ap);
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::WARNING, file, line, res);
            free(res);
        }

        void error(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            // 1.判断消息级别
            if (LogLevel::value::ERROR < limitLevel_)
                return;

            // 2. 提取不定参数
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed";
                va_end(ap);
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::ERROR, file, line, res);
            free(res);
        }

        void fatal(const std::string &file, size_t line, const std::string &fmt, ...)
        {
            // 1.判断消息级别
            if (LogLevel::value::FATAL < limitLevel_)
                return;

            // 2. 提取不定参数
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed";
                va_end(ap);
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::FATAL, file, line, res);
            free(res);
        }

    protected:
        void serialize(LogLevel::value level, const std::string &file, size_t line, char *res)
        {
            // 3. 构建LogMessage对象
            LogMessage msg(level, file, line, res, loggerName_);

            // 4.格式化
            std::stringstream ss;
            formmatter_->format(ss, msg);

            // 5. 日志落地
            log(ss.str().c_str(), ss.str().size());
        }
        virtual void log(const char *data, size_t len) = 0;

    protected:
        std::mutex mutex_;
        std::string loggerName_;
        std::atomic<LogLevel::value> limitLevel_;
        Formmatter::ptr formmatter_;
        std::vector<LogSink::ptr> sinks_;
    };

    /*同步日志器负责通过日志落地模块进行落地*/
    class SyncLogger : public Logger
    {
    public:
        SyncLogger(const std::string &loggerName, LogLevel::value limitLevel,
                   Formmatter::ptr &formmatter,
                   std::vector<LogSink::ptr> &sinks) : Logger(loggerName, limitLevel, formmatter, sinks)
        {
        }

    protected:
        void log(const char *data, size_t len) override
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (sinks_.empty())
                return;
            for (auto &sink : sinks_)
            {
                sink->log(data, len);
            }
        }
    };

    /*异步日志器*/
    class AsyncLogger : public Logger
    {
    public:
        AsyncLogger(const std::string &loggerName, LogLevel::value limitLevel,
                    Formmatter::ptr &formmatter,
                    std::vector<LogSink::ptr> &sinks, AsyncType looperType) : Logger(loggerName, limitLevel, formmatter, sinks),
                                                                              looper_(std::make_shared<AsyncLooper>(std::bind(&AsyncLogger::reLog,
                                                                                                                              this, std::placeholders::_1),
                                                                                                                    looperType))

        {
        }

    protected:
        // 将数据写入到缓冲区
        void log(const char *data, size_t len) override
        {
            looper_->push(data, len);
        }

        // 设计一个实际落地函数，将数据从缓冲区中落地
        void reLog(Buffer &buffer)
        {
            if (sinks_.empty())
                return;
            for (auto &sink : sinks_)
            {
                sink->log(buffer.begin(), buffer.readAbleSize());
            }
        }

    protected:
        AsyncLooper::ptr looper_;
    };

    /*使用建造者模式，降低使用户使用成本*/
    enum class LoggerType
    {
        LOGGER_SYNC,
        LOGGER_ASYNC
    };
    class LoggerBuilder
    {
    public:
        LoggerBuilder()
            : loggerType_(LoggerType::LOGGER_SYNC), limitLevel_(LogLevel::value::DEBUG), looperType_(AsyncType::ASYNC_SAFE)
        {
        }
        void buildLoggerType(LoggerType loggerType)
        {
            loggerType_ = loggerType;
        }
        void buildEnalleUnSafe()
        {
            looperType_ = AsyncType::ASYNC_UNSAFE;
        }
        void buildLoggerName(const std::string &loggerName)
        {
            loggerName_ = loggerName;
        }

        void buildLoggerLevel(LogLevel::value limitLevel)
        {
            limitLevel_ = limitLevel;
        }

        void buildLoggerFormmater(const std::string &pattern)
        {
            formmatter_ = std::make_shared<Formmatter>(pattern);
        }

        template <typename SinkType, typename... Args>
        void buildLoggerSink(Args &&...args)
        {
            LogSink::ptr psink = SinkFactory::create<SinkType>(std::forward<Args>(args)...);
            sinks_.push_back(psink);
        }
        virtual Logger::ptr build() = 0;

    protected:
        LoggerType loggerType_;
        std::string loggerName_;
        LogLevel::value limitLevel_;
        Formmatter::ptr formmatter_;
        std::vector<LogSink::ptr> sinks_;
        AsyncType looperType_;
    };

    /* 局部日志器建造者 */
    class LocalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(!loggerName_.empty()); // 必须有日志器名称

            if (formmatter_.get() == nullptr)
            {
                formmatter_ = std::make_shared<Formmatter>();
            }

            if (sinks_.empty())
            {
                buildLoggerSink<StdOutSink>();
            }

            if (loggerType_ == LoggerType::LOGGER_ASYNC)
            {
                return std::make_shared<AsyncLogger>(loggerName_, limitLevel_, formmatter_, sinks_, looperType_);
            }

            return std::make_shared<SyncLogger>(loggerName_, limitLevel_, formmatter_, sinks_);
        }
    };
};
