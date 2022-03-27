#include "../sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run in fiber begin";
    sylar::Fiber::GetThis()->swapOut();
    // sylar::Fiber::GetThis()->YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run in fiber end";
}


int main(int argc, char** argv) {
    sylar::Fiber::GetThis();  // 创建主Fiber，区别下面的个人fiber
    SYLAR_LOG_INFO(g_logger) << "main begin";
    sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));
    fiber->swapIn();
    SYLAR_LOG_INFO(g_logger) << "main after swapIn";
    fiber->swapIn();
    SYLAR_LOG_INFO(g_logger) << "main after end";
    return 0;
}
