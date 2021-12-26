#include "log.h"
#include<string>

namespace sylar {

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
        for(auto& i:m_appenders) {
            i->log(level, event);
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
};