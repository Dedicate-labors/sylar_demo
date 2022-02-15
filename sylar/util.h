/**
 * @file util.h
 * @brief 常用的工具函数
 * */
#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__

#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>

namespace sylar
{
    pid_t GetThreadId();
    uint32_t GetFiberId();
}

#endif
