#include "log.h"
#include<string>
#include<iostream>
#include<functional>
#include<map>
#include<tuple>
#include<time.h>

namespace sylar {

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   const char * file, int32_t line, uint32_t elapse,
                   pid_t thread_id, uint32_t fiber_id, uint64_t time)
:m_file(file)
,m_line(line)
,m_elapse(elapse)
,m_threadId(thread_id)
,m_fiberId(fiber_id)
,m_time(time)
,m_logger(logger)
,m_level(level)
{

}


LogEvent::~LogEvent() {

}


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
    #undef XX
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}


void LogEvent::format(const char * fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}


void LogEvent::format(const char * fmt, va_list al) {
    char * buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}


LogEventWrap::LogEventWrap(LogEvent::ptr e)
:m_event(e)
{

}


LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}


std::stringstream& LogEventWrap::getSs() {
    return m_event->getSs();
}


class MessageFormatItem: public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string& fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};


class LevelFormatItem: public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};


class ElapseFormatItem: public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }  
};


class LoggerNameFormatItem: public LogFormatter::FormatItem{
public:
    LoggerNameFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << logger->getName();
    }
};


class ThreadIdFormatItem: public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};


class FiberIdFormatItem: public LogFormatter::FormatItem{
public:
    FiberIdFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};


class NewLineFormatItem: public LogFormatter::FormatItem{
public:
    NewLineFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};


class DateTimeFormatItem: public LogFormatter::FormatItem{
public:
    DateTimeFormatItem(const std::string & format = "%Y-%m-%d %H:%M:%S"): m_format(format)
    {
            
    }
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        time_t now_time = event->getTime();
        struct tm * tp = localtime(&now_time);
        char buf[80];
        strftime(buf, sizeof(buf), m_format.c_str(), tp);
        os << buf;
    }
private:
    std::string m_format;
};


class FilenameFormatItem: public LogFormatter::FormatItem{
public:
    FilenameFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};


class LineFormatItem: public LogFormatter::FormatItem{
public:
    LineFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};


class StringFormatItem: public LogFormatter::FormatItem{
public:
    // 这个的str是一定要的
    StringFormatItem(const std::string & str):m_string(str) {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};


class TabFormatItem: public LogFormatter::FormatItem{
public:
    TabFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
};


Logger::Logger(const std::string& name):
m_name(name), m_level(LogLevel::DEBUG)
{
    // 这里就是给m_formatter设定一个默认值
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}


void Logger::addAppender(LogAppender::ptr appender){
    if (!appender->getFormatter()) {
        appender->setFormattter(m_formatter);
    }
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
    reopen();
}


void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        m_filestream << m_formatter->format(logger, level, event);
    }
}


bool FileLogAppender::reopen() {
    if (m_filestream.is_open()) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream; //双感叹号!!作用就是非0值转成1，而0值还是0.
}


void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        std::cout << m_formatter->format(logger, level, event);
    }
}


LogFormatter::LogFormatter(const std::string& pattern): m_pattern(pattern)
{
    init();
}


std::string LogFormatter::format(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto & i: m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}


/*
m_pattern 可能包含 %xx %xx{yy} %% 正常/错误字符串 5个情况
vec内元组含义:
1. 如果是正常/错误字符串, std::get<0>(vec[i])是字符串内容，std::get<1>(vec[i])是空，std::get<2>(vec[i])是0
2. 如果是正确格式字符串, std::get<0>(vec[i])是xx，std::get<1>(vec[i])是yy，std::get<2>(vec[i])是1
*/
void LogFormatter::init()
{
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        // m_pattern[i] == '%'的情况
        if (!nstr.empty()) {
            // 获取nstr
            vec.push_back(std::make_tuple(nstr, "", 0));
            // 清空nstr内容
            nstr.clear();
        }
        
        size_t next_i = i + 1;
        std::string str, fmt;
        if (next_i >= m_pattern.size()) { vec.push_back(std::make_tuple("<pattern_error>", "", 0)); break; }

        //next_i < m_pattern.size()的情况下，对m_pattern[next_i]的情况处理
        if (m_pattern[next_i] == '%') {
            // %%的特殊情况
            nstr.append(1, m_pattern[i]);
            i = next_i;
            continue;
        }
        
        //m_pattern[next_i] 不是字母
        if (!isalpha(m_pattern[next_i])) { 
            vec.push_back(std::make_tuple("<pattern_error>", "", 0));
            continue;
        }
        //%xx %xx{yy} 错误的格式 三种情况
        int fmt_status = 0;  //最后0是 %xx；2是%xx{yy}；1是错误格式
        int fmt_begin = 0;
        while(next_i < m_pattern.size()) {
            if(m_pattern[next_i] == '{') {
                // 取 % 到 { 间的str
                str = m_pattern.substr(i+1, next_i-i-1);
                fmt_begin = next_i;
                fmt_status = 1;
            }
            if(fmt_status == 1 && m_pattern[next_i] == '}') {
                //提取出fmt
                fmt = m_pattern.substr(fmt_begin + 1, next_i - fmt_begin - 1);
                fmt_status = 2;
                i = next_i;

                vec.push_back(std::make_tuple(str, fmt, 1));
                break;
            }
            if(fmt_status == 0) {
                if(!isalpha(m_pattern[next_i])) {
                    str = m_pattern.substr(i+1, next_i-i-1);
                    i = next_i - 1;

                    vec.push_back(std::make_tuple(str, fmt, 1));
                    break;
                }
                if(isalpha(m_pattern[next_i]) && next_i + 1 >= m_pattern.size()) {
                    str = m_pattern.substr(i+1, next_i);
                    i = next_i;

                    vec.push_back(std::make_tuple(str, fmt, 1));
                    break;
                }
            }
            ++next_i;
        }
        if(fmt_status == 1) { vec.push_back(std::make_tuple("<pattern_error>", "", 0)); i = next_i; }
    }

    // 最后可能的nstr
    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> m_format_items = 
    {
#define XX(str, C) \
    {#str, [](const std::string& fmt) {return fmt != "" ? FormatItem::ptr(new C(fmt)):FormatItem::ptr(new C()); }},
        
    XX(m, MessageFormatItem)        //m:消息
    XX(p, LevelFormatItem)          //p:日志级别
    XX(r, ElapseFormatItem)         //r:累计毫秒数
    XX(c, LoggerNameFormatItem)     //c:日志名称
    XX(t, ThreadIdFormatItem)       //t:线程id
    XX(n, NewLineFormatItem)        //n:换行
    XX(d, DateTimeFormatItem)       //d:时间
    XX(f, FilenameFormatItem)       //f:文件名
    XX(l, LineFormatItem)           //l:行号
    XX(T, TabFormatItem)            //T:Tab
    XX(F, FiberIdFormatItem)        //F:协程id
    // XX(N, ThreadNameFormatItem)     //N:线程名称
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
        
        // std::cout << "{" << std::get<0>(i) << "} - {" << std::get<1>(i) << "} - {" << std::get<2>(i)  << "}" << std::endl;
    }
}


LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
}


Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root : it->second;
}


void LoggerManager::init() {

}

}