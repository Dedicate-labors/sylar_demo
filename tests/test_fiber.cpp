#include "../sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run in fiber begin";
    sylar::Fiber::GetThis()->back();
    SYLAR_LOG_INFO(g_logger) << "run in fiber end";
}

void test_fiber() {
    sylar::Fiber::GetThis();  // 创建主Fiber，区别下面的个人fiber
    SYLAR_LOG_INFO(g_logger) << "main begin";
    sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber, 0, true));
    fiber->call();
    SYLAR_LOG_INFO(g_logger) << "main after swapIn";
    fiber->call();
    SYLAR_LOG_INFO(g_logger) << "main after end";
}


int main(int argc, char** argv) {
    sylar::Thread::SetName("main");

    std::vector<sylar::Thread::ptr> thrs;
    // for(int i = 0; i < 3; ++i) {
    thrs.push_back(sylar::Thread::ptr(new sylar::Thread(&test_fiber, "name_" + std::to_string(0))));
    // }
    for(auto i  : thrs) {
        i->join();
    }
    return 0;
}
