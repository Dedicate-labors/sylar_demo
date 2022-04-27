#include "log.h"
#include<string>
#include<iostream>
#include<functional>
#include<map>
#include<tuple>
#include<time.h>
#include<string.h>
#include "config.h"

namespace sylar {

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   const char * file, int32_t line, uint32_t elapse,
                   pid_t thread_id, uint32_t fiber_id, uint64_t time,
                   const std::string& thread_name)
:m_file(file)
,m_line(line)
,m_elapse(elapse)
,m_threadId(thread_id)
,m_fiberId(fiber_id)
,m_time(time)
,m_logger(logger)
,m_level(level)
,m_threadName(thread_name)
{

}


LogEvent::~LogEvent() {

}


// 接受LogLevel::Level，通过宏函数转换为const char * 类型
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


// 接受字符串类型，通过宏函数转换为LogLevel::Level
LogLevel::Level LogLevel::FromString(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
#define XX(level) \
    if(strcmp(str.c_str(), #level) == 0) return level;
    XX(DEBUG)
    XX(INFO)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
    return UNKNOWN;
#undef XX
}


// 将不定式参数通过格式化形成的字符串传入stringstream m_ss
void LogEvent::format(const char * fmt, ...) {
    va_list al;
    va_start(al, fmt);  // 选择自fmt后开始的不定参数
    format(fmt, al);
    va_end(al);  // 清空al指针
}


// 将va_list al通过格式化形成的字符串传入stringstream m_ss
void LogEvent::format(const char * fmt, va_list al) {
    char * buf = nullptr;
    // vasprintf专门负责va_list类型的sprintf
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
        os << event->getLogger()->getName();
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


class ThreadNameFormatItem: public LogFormatter::FormatItem{
public:
    ThreadNameFormatItem(const std::string & fmt = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadName();
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
    DateTimeFormatItem(const std::string & format = "%Y-%m-%d %H:%M:%S"): m_format(format) { }
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


Logger::Logger(const std::string& name)
:m_name(name)
,m_level(LogLevel::DEBUG)
{
    // 这里就是给m_formatter设定一个默认值
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    // m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%n%f:%l%n%m%n"));
    if(name == "root" && m_appenders.empty()) {
        // root logger 的appnder加的就是logger默认的fmt，所以算入appnder的fmt
        this->addAppender(LogAppender::ptr(new StdoutLogAppender));
    }
}


std::string Logger::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    if(m_level != LogLevel::UNKNOWN) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    for(auto& i : m_appenders) {
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}


void Logger::setFormatter(LogFormatter::ptr val) {
    MutexType::Lock lock(m_mutex);
    m_formatter = val;

    // 让fmt影响已经存在的appenders，但仅限该appender先天就使用的logger的fmt的情况
    for(auto& i : m_appenders) {
        MutexType::Lock ll(i->m_mutex);
        if(!i->m_hasFormatter) {
            i->m_formatter = m_formatter;
        }
    }
}


void Logger::setFormatter(const std::string& val) {
    sylar::LogFormatter::ptr new_val(new sylar::LogFormatter(val));
    if(new_val->isError()) {
        std::cout << "Logger setFormatter name=" << m_name
                  << " value=" << val << " invaild formatter"
                  << std::endl;
        return; 
    }
    // m_formatter = new_val;
    setFormatter(new_val);
}


LogFormatter::ptr Logger::getFormatter() {
    return m_formatter;
}


void Logger::addAppender(LogAppender::ptr appender){
    MutexType::Lock lock(m_mutex);
    if (!appender->getFormatter()) {
        MutexType::Lock ll(appender->m_mutex);
        // 因为是logger给的，所以不算入appender本身所有
        appender->m_formatter = m_formatter;
    }
    m_appenders.push_back(appender);
}


void Logger::delAppender(LogAppender::ptr appender){
    MutexType::Lock lock(m_mutex);
    for(auto it = m_appenders.begin(); it != m_appenders.end();
        ++it) {
            if(*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
}


void Logger::clearAppenders() {
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}


void Logger::log(LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level) {
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        if(!m_appenders.empty()) {
            // this本身有设置appender
            for(auto& i:m_appenders) {
                i->log(self, level, event);
            }
        } else if(m_root) {
            // 使用默认的m_root的
            m_root->log(level, event);
        }
        // 或者不输出日志
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


// 新设置的fmt直接生效
void LogAppender::setFormatter(LogFormatter::ptr val) {
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
    m_hasFormatter = m_formatter ? true:false;
}


// 新设置的fmt直接生效
void LogAppender::setFormatter(const std::string& fmt) {
    sylar::LogFormatter::ptr new_val(new sylar::LogFormatter(fmt));
    if(new_val->isError()) {
        std::cout << "LoggerAppender setFormatter, value=" 
                  << fmt << " invaild formatter" << std::endl;
        m_hasFormatter = false;
        return; 
    }
    setFormatter(new_val);
}


LogFormatter::ptr LogAppender::getFormatter() {
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}


FileLogAppender::FileLogAppender(const std::string& filename):
m_filename(filename)
{
    reopen();
}


void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        // 每隔段时间重新打开文件，避免文件被删除后，代码无法感知导致的问题
        uint64_t now = time(0);
        if(now != m_lastTime) {
            reopen();
            // 即使出现条件竞争，对重新打开文件这个操作影响不大
            m_lastTime = now;
        }
        MutexType::Lock lock(m_mutex);
        m_filestream << m_formatter->format(logger, level, event);
    }
}


std::string FileLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if(m_level != LogLevel::UNKNOWN) node["level"] = LogLevel::ToString(m_level);
    if(m_formatter && m_hasFormatter) node["formatter"] = m_formatter->getPattern();
    std::stringstream ss;
    ss << node;
    return ss.str();
}


bool FileLogAppender::reopen() {
    MutexType::Lock lock(m_mutex);
    if (m_filestream.is_open()) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream; //双感叹号!!作用就是非0值转成1，而0值还是0.
}


void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        MutexType::Lock lock(m_mutex);
        std::cout << m_formatter->format(logger, level, event);
    }
}


std::string StdoutLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(m_level != LogLevel::UNKNOWN) node["level"] = LogLevel::ToString(m_level);
    if(m_formatter && m_hasFormatter) node["formatter"] = m_formatter->getPattern();
    std::stringstream ss;
    ss << node;
    return ss.str();
}


// 当构造一个LogFormatter时必须传入pattern, 之后对其init()解析，解析结果放入m_items中
LogFormatter::LogFormatter(const std::string& pattern): m_pattern(pattern)
{
    init();
}


/*
params: logger指针, level, event指针
return：将日志格式化的string返回
*/
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
        if (next_i >= m_pattern.size()) { vec.push_back(std::make_tuple("<pattern_error>", "", 0)); m_error=true; break; }

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
            m_error=true;
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
        if(fmt_status == 1) { vec.push_back(std::make_tuple("<pattern_error>", "", 0)); m_error=true; i = next_i; }
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
    XX(N, ThreadNameFormatItem)     //N:线程名称
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
                m_error = true;
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
        
        // std::cout << "{" << std::get<0>(i) << "} - {" << std::get<1>(i) << "} - {" << std::get<2>(i)  << "}" << std::endl;
    }
}


LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    // m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
    m_loggers[m_root->m_name] = m_root;
    init();
}


Logger::ptr LoggerManager::getLogger(const std::string& name) {
    if(name == "") return m_root;
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()) {
        return it->second;
    }
    // 自己创建一个
    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;  // 添加了一个默认logger, 备用的
    m_loggers[name] = logger;
    return logger;
}


std::string LoggerManager::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for(auto& i : m_loggers ) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}


struct LogAppenderDefine {
    int type = 0; // 1 File, 2 Stdout
    LogLevel::Level level = LogLevel::UNKNOWN;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine& oth) const {
        return type == oth.type
        && level == oth.level
        && formatter == oth.formatter
        && file == oth.file;
    }
};

struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOWN;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine& oth) const {
        return name == oth.name
        && level == oth.level
        && formatter == oth.formatter
        && appenders == oth.appenders;
    }

    bool operator<(const LogDefine& oth) const {
        return name < oth.name;
    }
};

template<>
class LexicalCast<std::string, LogDefine> {
public:
    LogDefine operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        LogDefine ld;
        // 因为是从yml中取出很可能遇不到,所以IsDefined并且还要是对应类型
        if(!node["name"].IsDefined() || !node["name"].IsScalar()) {
            std::cout << "log config error: name is null or not is Scalar type, " << node
            << std::endl;
            // 如果Logger没有name就直接结束
            return ld;
        }
        ld.name = node["name"].as<std::string>();
        ld.level = LogLevel::FromString(node["level"].IsDefined() ? node["level"].as<std::string>().c_str() : "");
        if(node["formatter"].IsDefined()) {
            ld.formatter = node["formatter"].as<std::string>();
        }
        if(node["appenders"].IsDefined()) {
            for(size_t x = 0; x < node["appenders"].size(); ++x) {
                auto a = node["appenders"][x];
                if(!a["type"].IsDefined()) {
                    std::cout << "log config error: appender type is null, " << a
                              << std::endl;
                    continue;
                }
                std::string type = a["type"].as<std::string>();
                LogAppenderDefine lad;
                if(type == "FileLogAppender") {
                    lad.type = 1;
                    if(!a["file"].IsDefined()) {
                        std::cout << "log config error: fileappender file is null, " << a
                                  << std::endl;
                        continue;
                    }
                    lad.file = a["file"].as<std::string>();
                } else if(type == "StdoutLogAppender") {
                    lad.type = 2;
                } else {
                    std::cout << "log config error: appender type is invaild, " << a
                              << std::endl;
                    continue;
                }
                if(a["level"].IsDefined()) lad.level = LogLevel::FromString(a["level"].as<std::string>().c_str());
                if(a["formatter"].IsDefined()) lad.formatter = a["formatter"].as<std::string>();
                ld.appenders.push_back(lad);
            }
        }

        return ld;
    }
}; 


template<>
class LexicalCast<LogDefine, std::string> {
public:
    std::string operator() (const LogDefine& ld) {
        YAML::Node node;
        node["name"] = ld.name;
        node["level"] = LogLevel::ToString(ld.level);
        // ld.formatter 可能是空
        node["formatter"] = ld.formatter;
        for(auto& it:ld.appenders ) {
            YAML::Node na;
            if(it.type == 1) {
                na["type"] = "FileLogAppender";
                na["file"] = it.file;
            } else if(it.type == 2) {
                na["type"] = "StdoutLogAppender";
            }
            na["level"] = LogLevel::ToString(it.level);
            na["formatter"] = it.formatter;
            node["appenders"].push_back(na);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

sylar::ConfigVar<std::set<LogDefine>>::ptr g_log_defines = 
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

// 增加回调函数，表面上是比较std::set<LogDefine>的不同，实际上涉及logger的创建
struct LogIniter {
    LogIniter() {
        g_log_defines->addListener([](const std::set<LogDefine>& old_value, 
                const std::set<LogDefine>& new_value){
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "on_logger_conf_changed";
            for(auto& i : new_value) {
                // find只是使用了LogDefine::operator<，所以查找比较的是name值
                // auto it = old_value.find(i);
                sylar::Logger::ptr logger;
                // 新增logger || 修改logger
                logger = SYLAR_LOG_NAME(i.name);
                logger->setLevel(i.level);
                // formatter的配置，后面可能会删除
                if(!i.formatter.empty()) {
                    logger->setFormatter(i.formatter);
                }
                // appender的配置
                logger->clearAppenders(); // 理论上来说logger新建出来，除了root的logger，其他都是无appender的
                for(auto& a: i.appenders) {
                    sylar::LogAppender::ptr ap;
                    if(a.type == 1) {
                        ap.reset(new FileLogAppender(a.file));
                    } else if(a.type == 2) {
                        ap.reset(new StdoutLogAppender);
                    }
                    ap->setLevel(a.level);
                    // 如果ap没有fmt，那么使用logger本身默认的fmt
                    if(!a.formatter.empty()) ap->setFormatter(a.formatter);
                    logger->addAppender(ap);
                }
            }

            for(auto& i : old_value) {
                auto it = new_value.find(i);
                if(it == new_value.end()) {
                    // 删除的logger
                    auto logger = SYLAR_LOG_NAME(i.name);
                    logger->setLevel((LogLevel::Level)(100));
                    logger->clearAppenders(); // 只能使用logger备用的m_root进行log
                }
            }
        });
    }
};

static LogIniter __log_init;

void LoggerManager::init() {

}

}