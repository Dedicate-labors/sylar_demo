#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include<string>
#include<stdint.h>
#include<memory>
#include<list>

namespace sylar {

//一个日志就是一个logevent, 日志事件
class LogEvent {
public:
typedef std::shared_ptr<LogEvent> ptr; 
    LogEvent();
private:
    const char * m_file = nullptr;  //文件名
    int32_t m_line = 0;             //符号
    uint32_t m_elapse = 0;          //程序启动开始到现在的毫秒数
    int32_t m_threadid = 0;         //线程id
    uint32_t m_fiberid = 0;         //协程id
    uint32_t m_time = 0;            //时间戳
    std::string m_content;
};


//日志级别
class LogLevel {
public:
    enum Level {
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
};

//日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;

    std::string format(LogEvent::ptr event);
private:
};

//日志输出目的地
class  LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr; 
    virtual ~LogAppender() {}

    void log(LogLevel::Level level, LogEvent::ptr event);
private:
    LogLevel::Level m_level;
};

//日志器 
class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);
    
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }
private:
    std::string m_name;                         //日志名称
    LogLevel::Level m_level;                    //日志级别
    std::list<LogAppender::ptr>  m_appenders;   //Appender集合
};


// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {

};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {

};

};

#endif