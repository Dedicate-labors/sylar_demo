#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include<string>
#include<stdint.h>
#include<memory>
#include<list>
#include<sstream>
#include<fstream>
#include<vector>
#include<cstdarg>
#include<map>
#include "singleton.h"
#include "util.h"
#include "thread.h"

/*
params: 接受logger指针和level, 构造默认LogEvent以及LogEventWrap
return: LogEventWrap管理的LogEvent的stringstream m_ss

目的：希望通过传入的logger打印一条level级别的日志
原理：LogEventWrap对象析构时将调用传入LogEvent的logger对象的log方法打印LogEvent的信息
*/
#define SYLAR_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level,\
                            __FILE__, __LINE__, 0, sylar::GetThreadId(),\
                            sylar::GetFiberId(), time(0)))).getSs()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)


/*
params: 接受logger指针和level, fmt, 以及不定参数，构造默认LogEvent以及LogEventWrap
return: LogEventWrap管理的LogEvent的format方法(fmt, 不定参数)

目的：通过logger把(不定参数+fmt)形成的字符串输出为level级别的日志
原理：(不定参数+fmt)形成的字符串写入LogEvent的stringstream m_ss中，LogEventWrap对象析构时
将调用传入LogEvent的logger对象的log方法打印LogEvent的信息
*/
#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                            __FILE__, __LINE__, 0, sylar::GetThreadId(),\
                            sylar::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance().getRoot()
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance().getLogger(name)

namespace sylar {

// 前置类声明
class Logger;
class LoggerManager;


//日志级别：辅助类，可以默认构造
class LogLevel {
public:
    enum Level {
        UNKNOWN = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char * ToString(LogLevel::Level level);
    static LogLevel::Level FromString(std::string str);
};


// 日志事件：LogEvent主要负责保存和返回日志信息，构造参数众多，无默认参数
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr; 
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
            const char * file, int32_t line, uint32_t elapse,
            pid_t thread_id, uint32_t fiber_id, uint64_t time);
    ~LogEvent();
    const char * getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    pid_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    const std::string getContent() const { return m_ss.str(); }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() const { return m_level; }

    std::stringstream& getSs() { return m_ss; }
    void format(const char * fmt, ...);
    void format(const char *fmt, va_list al);
private:
    const char * m_file = nullptr;      //文件名
    int32_t m_line = 0;                 //符号
    uint32_t m_elapse = 0;              //程序启动开始到现在的毫秒数
    pid_t m_threadId;                   //线程id
    uint32_t m_fiberId = 0;             //协程id
    uint64_t m_time = 0;                //时间戳
    std::stringstream m_ss;             //日志内容
    std::shared_ptr<Logger> m_logger;   //打印日志的logger指针
    LogLevel::Level m_level;            //日志level
};


/*
事件包装：必须传入LogEvent进行构造
对LogEvent对象的简单包装，主要功能是在析构时使用LogEvent的logger打印日志
*/
class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    LogEvent::ptr getEvent() const { return m_event; }
    std::stringstream& getSs();
private:
    LogEvent::ptr m_event;
};


//日志格式器: 必须传入符合规则的pattern并对其解析(init()函数)，最后保存在m_items对象中
//formatter生成后不会修改，只可能被覆盖，所以不需要加锁
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);
    // 返回字符串类型的pattern，即日志格式
    std::string getPattern() { return m_pattern; }
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        FormatItem(const std::string& fmt = ""){};
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };
    void init();

    bool isError() const { return m_error; }
private:    
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false; // 默认无错
};

//日志输出目的地
class  LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Spinlock MutexType;
    virtual ~LogAppender() {}
    LogAppender(){ m_level = LogLevel::DEBUG; }

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& fmt);
    LogFormatter::ptr getFormatter();
    void setLevel(LogLevel::Level level) { m_level = level; }
    LogLevel::Level getLevel() { return  m_level; }

protected:
    LogLevel::Level m_level;
    bool m_hasFormatter = false;  // 判断appender是否具备自己的fmt，logger给的默认fmt不算入
    MutexType m_mutex;
    LogFormatter::ptr m_formatter;
};

//日志器 
class Logger: public std::enable_shared_from_this<Logger> {
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;

    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);
    
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }
    const std::string & getName() const { return m_name; }
    // 设置logger本身的默认formatter
    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter();
    std::string toYamlString();
private:
    std::string m_name;                         //日志名称
    LogLevel::Level m_level;                    //日志级别
    MutexType m_mutex;
    std::list<LogAppender::ptr>  m_appenders;   //Appender集合
    LogFormatter::ptr m_formatter;              //不使用Appender的默认情况
    Logger::ptr m_root;                         // 备用logger
};


// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
private:
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;

    //重新打开文件，文件打开成功返回true
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
    uint64_t m_lastTime = 0;  // 没隔几秒重新打开文件
};


// LoggerManager注意是单例
class LoggerManager {
public:
    typedef Spinlock MutexType;
    LoggerManager();
    Logger::ptr getRoot() const { return m_root; }
    Logger::ptr getLogger(const std::string& name);
    std::string toYamlString();
    void init();
private:
    MutexType m_mutex;
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

typedef sylar::Singleton<LoggerManager> LoggerMgr;
}
#endif