/**
 * @file util.h
 * @brief 常用的工具函数
 * */
#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__

#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <execinfo.h>

namespace sylar
{
    pid_t GetThreadId();

    uint32_t GetFiberId();

    void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);

    std::string BacktraceToString(int size, int skip = 2, const std::string& prefix="");
}

#endif
