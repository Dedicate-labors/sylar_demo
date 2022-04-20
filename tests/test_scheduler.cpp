#include "../sylar/sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void cb_func2() {
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        sylar::Scheduler::GetThis()->schedule(&cb_func2, sylar::GetThreadId());
    }
}

void test_scheduler() {
    SYLAR_LOG_INFO(g_logger) << "main";
    sylar::Scheduler sc(3, false, "test");
    sc.start();
    sleep(2);
    SYLAR_LOG_INFO(g_logger) << "schedule";
    sc.schedule(cb_func2);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "over";
}

void cb_func1() {
    SYLAR_LOG_INFO(g_logger) << "test_fiber";
}

void test_cb() {
    SYLAR_LOG_INFO(g_logger) << "test_cb start";
    sylar::Scheduler sc;
    sc.schedule(cb_func1);
    sc.start();
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "test_cb over";
}

int main(int argc, char** argv) {
    // test_cb();
    test_scheduler();
    return 0;
}