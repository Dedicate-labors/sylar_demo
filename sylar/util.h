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

namespace sylar
{
// 线程/协程 id相关
pid_t GetThreadId();
uint32_t GetFiberId();

// 栈信息
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix="");

// 时间相关
uint64_t GetCurrentMS();   // 毫秒
uint64_t GetCurrentUS();   // 微秒
}

#endif
