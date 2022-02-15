#include<iostream>
#include "../sylar/log.h"
#include "../sylar/util.h"

int main(int argc, char ** argv) {
    sylar::Logger::ptr logger(new sylar::Logger);
    sylar::StdoutLogAppender::ptr stdout_appender(new sylar::StdoutLogAppender());
    // stdout_appender->setLevel(sylar::LogLevel::DEBUG);
    logger->addAppender(stdout_appender);
    

    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T {%p} %t%m%n"));
    file_appender->setLevel(sylar::LogLevel::ERROR);
    file_appender->setFormattter(fmt);
    logger->addAppender(file_appender);


    SYLAR_LOG_INFO(logger) << "test marco";
    SYLAR_LOG_ERROR(logger) << "error";
    SYLAR_LOG_FMT_INFO(logger, "%s %d %d", "aa:", 223, 520);

    auto l = sylar::LoggerMgr::GetInstance().getLogger("ZXL");  // 这里是没有init出ZXL的logger的
    SYLAR_LOG_ERROR(l) << "My name is ZXL";
    return 0;
}