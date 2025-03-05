#pragma once
/*
    日志落地模块的实现
        1. 抽象落地基类
        2. 派生子类
        3. 使用工厂模式进行创建与表示的分离
*/
#include "util.hpp"
#include <memory>
#include <fstream>
#include <cassert>
#include <sstream>
namespace mylog
{
    class LogSink
    {
    public:
        using ptr = std::shared_ptr<LogSink>;
        LogSink() {}
        virtual ~LogSink() {}
        virtual void log(const char *data, size_t len) = 0;
    };

    // 标准输出
    class StdOutSink : public LogSink
    {
    public:
        void log(const char *data, size_t len) override
        {
            std::cout.write(data, len);
        }
    };

    // 指定文件
    class FileSink : public LogSink
    {
    public:
        FileSink(const std::string &pathname)
            : pathname_(pathname)
        {
            // 1.创建日志文件所用的路径
            File::createDiretory(File::path(pathname));

            // 2. 创建并打开日志文件
            ofs_.open(pathname_, std::ios::binary | std::ios::app);
            assert(ofs_.is_open());
        }
        void log(const char *log, size_t len) override
        {
            ofs_.write(log, len);
            assert(ofs_.good());
        }

    protected:
        std::string pathname_;
        std::ofstream ofs_;
    };

    // 滚动文件(以大小)
    class RollBySizeSink : public LogSink
    {
    public:
        RollBySizeSink(const std::string &basename, size_t maxSize)
            : basename_(basename), maxSize_(maxSize), curSize_(0), nameCount(0)
        {
            // 1.创建日志文件所用的路径
            std::string pathname = createNewFile();
            File::createDiretory(File::path(pathname));

            // 2. 创建并打开日志文件
            ofs_.open(pathname, std::ios::binary | std::ios::app);
            assert(ofs_.is_open());
        }

        void log(const char *log, size_t len) override
        {
            if (curSize_ >= maxSize_)
            {
                ofs_.close();
                std::string pathname = createNewFile();
                ofs_.open(pathname, std::ios::binary | std::ios::app);
                curSize_ = 0;
                assert(ofs_.is_open());
            }
            ofs_.write(log, len);
            curSize_ += len;
            assert(ofs_.good());
        }

    protected:
        // 进行大小判断，超过就创建新文件
        std::string createNewFile()
        {
            time_t t = Date::getCurrentTime();
            struct tm lt;
            localtime_r(&t, &lt);
            std::stringstream filename;
            filename << basename_ << (lt.tm_year + 1900) << (lt.tm_mon + 1) << lt.tm_mday
                     << lt.tm_hour << lt.tm_min << lt.tm_sec << "-" << nameCount++ << ".log";
            return filename.str();
        }

    protected:
        std::string basename_; // 基础文件 + 扩展文件名
        std::ofstream ofs_;
        size_t maxSize_; // 最大文件大小
        size_t curSize_;
        size_t nameCount; // 名称计数器
    };

    // 简单工厂模式
    class SinkFactory
    {
    public:
        template <typename SinkType, typename... Args>
        static LogSink::ptr create(Args &&...args)
        {
            return std::make_shared<SinkType>(std::forward<Args>(args)...);
        }
    };
};