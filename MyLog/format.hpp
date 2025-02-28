#pragma once
#include "message.hpp"
#include <ctime>
#include <memory>
namespace mylog
{
    /*抽象格式基类*/
    class FormatItem
    {
    public:
        using prt = std::shared_ptr<FormatItem>;
        virtual void format(std::ostream &out, LogMessage &msg) = 0;
    };

    /*派生格式化子类--消息，等级，时间，文件名，行号，线程ID，日志器名，制表符，换行，其他*/
    class MessageFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, LogMessage &msg) override
        {
            out << msg.payload_;
        }
    };

    class LevelFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, LogMessage &msg) override
        {
            out << LogLevel::toString(msg.level_);
        }
    };

    static const std::string timeFormatDefault = "%H-%M-%S"; // 默认时间输出格式
    class TimeFormatItem : public FormatItem
    {
    public:
        TimeFormatItem(std::string timeFormat = timeFormatDefault)
            : timeFormat_(timeFormat)
        {
        }
        void format(std::ostream &out, LogMessage &msg) override
        {
            char buffer[40] = {0};
            strftime(buffer, sizeof(buffer) - 1, timeFormat_.c_str(), localtime(&msg.curtime_));
        }

    protected:
        std::string timeFormat_;
    };

    class FileFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, LogMessage &msg) override
        {
            out << msg.file_;
        }
    };

    class LineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, LogMessage &msg) override
        {
            out << msg.line_;
        }
    };

    class TreadIdFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, LogMessage &msg) override
        {
            out << msg.tid_;
        }
    };

    class LoggerFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, LogMessage &msg) override
        {
            out << msg.logger_;
        }
    };

    class TabFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, LogMessage &msg) override
        {
            out << '\t';
        }
    };

    class NLineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, LogMessage &msg) override
        {
            out << '\n';
        }
    };

    class OtherFormatItem : public FormatItem
    {
    public:
        OtherFormatItem(const std::string &str)
            : str_(str)
        {
        }
        void format(std::ostream &out, LogMessage &msg) override
        {
            out << str_;
        }

    protected:
        std::string str_;
    };

};