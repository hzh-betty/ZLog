#pragma once
#include "message.hpp"
#include <ctime>
#include <memory>
#include <vector>
#include <sstream>
#include <cassert>
namespace mylog
{
    /*抽象格式基类*/
    class FormatItem
    {
    public:
        using prt = std::shared_ptr<FormatItem>;
        virtual void format(std::ostream &out, const LogMessage &msg) = 0;
    };

    /*派生格式化子类--消息，等级，时间，文件名，行号，线程ID，日志器名，制表符，换行，其他*/
    class MessageFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMessage &msg) override
        {
            out << msg.payload_;
        }
    };

    class LevelFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMessage &msg) override
        {
            out << LogLevel::toString(msg.level_);
        }
    };

    static const std::string timeFormatDefault = "%H:%M:%S"; // 默认时间输出格式
    class TimeFormatItem : public FormatItem
    {
    public:
        TimeFormatItem(std::string timeFormat = timeFormatDefault)
            : timeFormat_(timeFormat)
        {
        }
        void format(std::ostream &out, const LogMessage &msg) override
        {
            char buffer[40] = {0};
            strftime(buffer, sizeof(buffer) - 1, timeFormat_.c_str(), localtime(&msg.curtime_));
            out << buffer;
        }

    protected:
        std::string timeFormat_;
    };

    class FileFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMessage &msg) override
        {
            out << msg.file_;
        }
    };

    class LineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMessage &msg) override
        {
            out << msg.line_;
        }
    };

    class TreadIdFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMessage &msg) override
        {
            out << msg.tid_;
        }
    };

    class LoggerFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMessage &msg) override
        {
            out << msg.logger_;
        }
    };

    class TabFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMessage &msg) override
        {
            out << "\t";
        }
    };

    class NLineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMessage &msg) override
        {
            out << "\n";
        }
    };

    class OtherFormatItem : public FormatItem
    {
    public:
        OtherFormatItem(const std::string &str)
            : str_(str)
        {
        }
        void format(std::ostream &out, const LogMessage &msg) override
        {
            out << str_;
        }

    protected:
        std::string str_;
    };

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

    class Formmatter
    {
    public:
        using ptr = std::shared_ptr<Formmatter>;
        Formmatter(const std::string &pattern = "[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n")
            : pattern_(pattern)
        {
            assert(parsePattern());
        }

        void format(std::ostream &out, const LogMessage &msg)
        {
            for (auto &item : items_)
            {
                item->format(out, msg);
            }
        }

        std::string format(const LogMessage &msg)
        {
            std::stringstream ss;
            for (auto &item : items_)
            {
                item->format(ss, msg);
            }
            return ss.str();
        }

    protected:
        // 对格式化字符串进行解析
        bool parsePattern()
        {
            std::vector<std::pair<std::string, std::string>> fmt_order;
            size_t pos = 0;
            std::string key, val;
            size_t n = pattern_.size();
            while (pos < n)
            {
                // 1. 不是%字符
                if (pattern_[pos] != '%')
                {
                    val.push_back(pattern_[pos++]);
                    continue;
                }

                // 2.是%%--转换为%字符
                if (pos + 1 < n && pattern_[pos + 1] == '%')
                {
                    val.push_back('%');
                    pos += 2;
                    continue;
                }

                // 3. 如果起始不是%添加
                if (!val.empty())
                {
                    fmt_order.push_back({"", val});
                    val.clear();
                }

                // 4. 开始处理格式化字符
                if (++pos == n)
                {
                    std::cerr << "%之后没有格式化字符" << std::endl;
                    return false;
                }

                key = pattern_[pos];
                pos++;

                // 5. 处理子规则字符
                if (pos < n && pattern_[pos] == '{')
                {
                    pos++;
                    while (pos < n && pattern_[pos] != '}')
                    {
                        val.push_back(pattern_[pos++]);
                    }

                    if (pos == n)
                    {
                        std::cerr << "未找到匹配的子规则字符 }" << std::endl;
                        return false;
                    }
                    pos++;
                }

                // 6. 插入对应的key与value
                fmt_order.push_back({key, val});

                key.clear();
                val.clear();
            }

            // 7. 添加对应的格式化子对象
            for (auto &item : fmt_order)
            {
                items_.push_back(createItem(item.first, item.second));
            }

            return true;
        }

        // 根据不同格式创建不同的格式化子项对象
        FormatItem::prt createItem(const std::string &key, const std::string &val)
        {

            if (key == "d")
                return std::make_shared<TimeFormatItem>(val);
            else if (key == "t")
                return std::make_shared<TreadIdFormatItem>();
            else if (key == "c")
                return std::make_shared<LoggerFormatItem>();
            else if (key == "f")
                return std::make_shared<FileFormatItem>();
            else if (key == "l")
                return std::make_shared<LineFormatItem>();
            else if (key == "p")
                return std::make_shared<LevelFormatItem>();
            else if (key == "T")
                return std::make_shared<TabFormatItem>();
            else if (key == "m")
                return std::make_shared<MessageFormatItem>();
            else if (key == "n")
                return std::make_shared<NLineFormatItem>();

            if (!key.empty())
            {
                std::cerr << "没有对应的格式化字符: %" << key << std::endl;
                abort();
            }
            return std::make_shared<OtherFormatItem>(val);
        }

    protected:
        std::string pattern_;
        std::vector<FormatItem::prt> items_;
    };
};