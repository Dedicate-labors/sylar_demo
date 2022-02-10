#include "log.h"
#include<string>
#include<iostream>
#include<functional>
#include<map>
#include<tuple>

namespace sylar {

const char * LogLevel::ToString(LogLevel::Level level) {
    switch (level)
    {
    #define XX(name) \
        case LogLevel::name: \
            return #name; \
            break;
    XX(DEBUG)
    XX(INFO)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
    #undef xx
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}


/*
%m -- 消息体
%p -- 日志Level
%r -- 输出该log前的耗时毫秒数
%c -- 日志名称
%t -- 线程名
%n -- 回车
%d -- 时间
%f -- 文件名
%l -- 行号
*/
class MessageFormatItem: public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string & fmt):LogFormatter::FormatItem(fmt) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};


class LevelFormatItem: public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string & fmt):LogFormatter::FormatItem(fmt) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};


class ElapseFormatItem: public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string & fmt):LogFormatter::FormatItem(fmt) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }  
};


class LoggerNameFormatItem: public LogFormatter::FormatItem{
public:
    LoggerNameFormatItem(const std::string & fmt):LogFormatter::FormatItem(fmt) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << logger->getName();
    }
};


class ThreadIdFormatItem: public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string & fmt):LogFormatter::FormatItem(fmt) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};


class FiberIdFormatItem: public LogFormatter::FormatItem{
public:
    FiberIdFormatItem(const std::string & fmt):LogFormatter::FormatItem(fmt) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};


class NewLineFormatItem: public LogFormatter::FormatItem{
public:
    NewLineFormatItem(const std::string & fmt):LogFormatter::FormatItem(fmt) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};


class DateTimeFormatItem: public LogFormatter::FormatItem{
public:
    DateTimeFormatItem(const std::string & format = "%Y:%m:%D %H:%M:%S"):m_format(format)
    {

    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getTime();
    }
private:
    std::string m_format;
};


class FilenameFormatItem: public LogFormatter::FormatItem{
public:
    FilenameFormatItem(const std::string & fmt):LogFormatter::FormatItem(fmt) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};


class LineFormatItem: public LogFormatter::FormatItem{
public:
    LineFormatItem(const std::string & fmt):LogFormatter::FormatItem(fmt) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};


class StringFormatItem: public LogFormatter::FormatItem{
public:
    StringFormatItem(const std::string & str):
    m_string(str), LogFormatter::FormatItem(str) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};


Logger::Logger(const std::string& name = "root"):
m_name(name)
{

}


void Logger::addAppender(LogAppender::ptr appender){
    m_appenders.push_back(appender);
}


void Logger::delAppender(LogAppender::ptr appender){
    for(auto it = m_appenders.begin(); it != m_appenders.end();
        ++it) {
            if(*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
}


void Logger::log(LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level) {
        auto self = shared_from_this();
        for(auto& i:m_appenders) {
            i->log(self, level, event);
        }
    }
}


void Logger::debug(LogEvent::ptr event){
    log(LogLevel::DEBUG, event);
}


void Logger::info(LogEvent::ptr event){
    log(LogLevel::INFO, event);
}


void Logger::warn(LogEvent::ptr event){
    log(LogLevel::WARN, event);
}


void Logger::error(LogEvent::ptr event){
    log(LogLevel::ERROR, event);
}


void Logger::fatal(LogEvent::ptr event){
    log(LogLevel::FATAL, event);
}


FileLogAppender::FileLogAppender(const std::string& filename):
m_filename(filename)
{

}


void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        m_filestream << m_formatter->format(logger, level, event);
    }
}


bool FileLogAppender::reopen() {
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream; //双感叹号!!作用就是非0值转成1，而0值还是0.
}


void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        std::cout << m_formatter->format(logger, level, event);
    }
}


LogFormatter::LogFormatter(const std::string& pattern): m_pattern(pattern)
{

}


std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto & i: m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}


/*
%xx %xx{yy} %% 三个类型，str表示xx的情况，type=1，str非xx的情况，type=0
m_pattern是上面类型的混合字符串，也可能是错误字符串
xx表示str, yy表示fotmat, type 是 int
从m_pattern中提取<str  format  type> 
*/
void LogFormatter::init()
{
    std::vector<std::tuple<std::string, std::string, int> > vec;
    std::string nstr; // 非格式含义的正常字符串
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        // m_pattern[i] == '%'后的情况
        if ((i + 1) < m_pattern.size()) {
            //双 %% 情况
            if (m_pattern[i+1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        // %xx %xx{yy} 情况
        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while(n < m_pattern.size()) {
            if (isspace(m_pattern[n])) {
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '{') {
                    // 取 % 到 { 间的str
                    str = m_pattern.substr(i+1, n-i-1);
                    fmt_status = 1; //解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }
            if (fmt_status == 1) {
                // 解析%xx{yy}中的yy，即fmt
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 2;
                    break;
                }
            }
        }
        
        if (!nstr.empty()) {
            vec.push_back(std::make_tuple(nstr, "", 0));
            nstr.clear(); // 清空它
        }
        if (fmt_status == 0) {
            str = m_pattern.substr(i+1, n - i - 1);
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<pattern_error>", fmt, 0));
        } else if (fmt_status == 2) {
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        }
    }

    // 最后可能的nstr
    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    // vec很可能变成内部成员
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> m_format_items = 
    {
#define XX(str, C) \
    {#str, [](const std::string& fmt) {return FormatItem::ptr(new C(fmt));}},
        
    XX(m, MessageFormatItem)
    XX(p, LevelFormatItem)
    XX(r, ElapseFormatItem)
    XX(c, LoggerNameFormatItem)
    XX(t, ThreadIdFormatItem)
    XX(n, NewLineFormatItem)
    XX(d, DateTimeFormatItem)
    XX(f, FilenameFormatItem)
    XX(l, LineFormatItem)
#undef XX
    };
    for(auto& i : vec) {
        if(std::get<2>(i) == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else if (std::get<2>(i) == 1) {
            // %xx  %xx{yy} 两种情况
            auto it = m_format_items.find(std::get<0>(i)); // 也有可能是不存在的it
            if(it == m_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format  %" + std::get<0>(i) + ">>")));
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
        
        std::cout << std::get<0>(i) << " - " << std::get<1>(i) << " - " << std::get<2>(i) << std::endl;
    }
}
}