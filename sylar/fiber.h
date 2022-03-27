#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>
#include "thread.h"

namespace sylar {

class Fiber: public std::enable_shared_from_this<Fiber> {
public:
    typedef std::shared_ptr<Fiber> ptr;

    // Fiber状态
    enum State {
        INIT,
        READY,
        EXEC,
        HOLD,
        TERM,
        EXCEPT,
    };
private:
    // 默认主协程创建
    Fiber();

public:
    // 子协程创建
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    // 协程析构
    ~Fiber();

    // 重置协程函数，并重置Fiber状态(INIT, TERM, EXCEPT) --> (INIT)
    void reset(std::function<void()> cb);
    // 切换到当前协程执行
    void swapIn();
    // 切换到后台执行
    void swapOut();
    // 协程id
    uint64_t getId() const { return m_id; }

public:
    // 下面是静态函数，负责和静态全局变量交互管理协程切换，注意和上面的子协程区分逻辑
    // 设置当前协程
    static void SetThis(Fiber* f);
    // 返回当前协程，没有就创建 主&子 协程再返回
    static Fiber::ptr GetThis();
    // 当前协程切换到后台，并且设置为Ready状态
    static void YieldToReady();
    // 当前协程切换到后台，并且设置为Hold状态
    static void YieldToHold();
    // 总协程数
    static uint64_t TotalFibers();
    // 主体函数，切换m_state
    static void MainFunc();
    // 返回当前协程的id
    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0;
    ucontext_t m_ctx;

    uint64_t m_stacksize = 0;
    void* m_stack = nullptr;

    State m_state = INIT;
    std::function<void()> m_cb;
};

}

#endif