#include<iostream>
#include "../sylar/log.h"
#include "../sylar/util.h"

void test_error_log() {
    auto l = sylar::LoggerMgr::GetInstance().getRoot();
    SYLAR_LOG_ERROR(l) << "test_error_log";
}

void test_fmt_error_log() {
    auto l = sylar::LoggerMgr::GetInstance().getRoot();
    SYLAR_LOG_FMT_ERROR(l, "%s %d %d", "test_fmt_error_log:", 223, 520);
}

void test_log_level() {
    auto l = sylar::LoggerMgr::GetInstance().getRoot();
    l->setLevel(sylar::LogLevel::ERROR);
    SYLAR_LOG_INFO(l) << "test info test_log_level";
    SYLAR_LOG_ERROR(l) << "test error test_log_level";
}


void test_new_error_log() {
    auto l = sylar::LoggerMgr::GetInstance().getRoot();
    sylar::StdoutLogAppender::ptr stdout_appender(new sylar::StdoutLogAppender());
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T {%p} %t%m%n"));
    stdout_appender->setFormatter(fmt);
    l->addAppender(stdout_appender);
    SYLAR_LOG_ERROR(l) << "test test_new_error_log log";
}

void test_new_appender_log() {
    auto l = sylar::LoggerMgr::GetInstance().getRoot();
    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    l->addAppender(file_appender);
    SYLAR_LOG_ERROR(l) << "test test_new_appender_log log";
}

void test_get_logger() {
    auto l = sylar::LoggerMgr::GetInstance().getLogger("ZXL");  // 之前没有称为ZXL的logger
    SYLAR_LOG_ERROR(l) << "My name is ZXL";
}

int main(int argc, char ** argv) {
    test_error_log();
    test_fmt_error_log();
    test_log_level();
    test_new_error_log();
    test_new_appender_log();
    test_get_logger();
    return 0;
}