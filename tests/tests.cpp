#include<iostream>
#include "../sylar/log.h"

int main(int argc, char ** argv) {
    sylar::Logger::ptr logger(new sylar::Logger);
    sylar::StdoutLogAppender::ptr stdout_appender(new sylar::StdoutLogAppender());
    // stdout_appender->setLevel(sylar::LogLevel::DEBUG);
    logger->addAppender(stdout_appender);
    
    // sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    // sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%t%p%t%m%n"));
    // file_appender->setFormattter(fmt);
    // file_appender->setLevel(sylar::LogLevel::ERROR);
    // logger->addAppender(file_appender);

    sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0)));
    event->getSs() << "hello MyName is ZXL";
    logger->log(sylar::LogLevel::DEBUG, event);
    std::cout << "hello sylar log" << std::endl;
    return 0;
}