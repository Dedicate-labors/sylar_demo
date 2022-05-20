#include "../sylar/sylar.h"
#include "../sylar/hook.h"

// 虽然没导入hook文件，但因为CMakeLists.txt 生成文件是的库连接，所以会使用hook的sleep，这是错误的
// Schedule模块还无法使用hook函数，IOManager模块才能正确使用

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void cb_func2() {
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    // sleep(1);
    if(--s_count >= 0) {
        sylar::Scheduler::GetThis()->schedule(&cb_func2, sylar::GetThreadId());
    }
}

void test_scheduler() {
    SYLAR_LOG_INFO(g_logger) << "main";
    // sylar::Scheduler sc(3, false, "test");
    sylar::Scheduler sc(3, true, "test");
    sc.start();
    // sleep(2);
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
    sylar::Scheduler sc(1, false);
    sc.start();

    sleep_f(15);
    sc.schedule(cb_func1);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "test_cb over";
}

int main(int argc, char** argv) {
    // test_cb();
    test_scheduler();
    return 0;
}