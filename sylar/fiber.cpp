#include "fiber.h"
#include "config.h"
#include "macro.h"
#include <atomic>

namespace sylar {
static std::atomic<uint64_t> s_fiber_id(0);
static std::atomic<uint64_t> s_fiber_count(0);
static thread_local Fiber* t_fiber = nullptr; // 表示目前协程
static thread_local Fiber::ptr t_threadFiber = nullptr; // master协程

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size", 1024*1024,"fiber stack size");

class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    // 如果使用mmap创建变量，届时使用munmap释放需要size传入
    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

Fiber::Fiber() {
    // 默认构造主协程
    m_state = EXEC;
    SetThis(this);
    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }
    ++s_fiber_count;
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize)
:m_id(++s_fiber_id)
,m_cb(cb)
{
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
}

Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        SYLAR_ASSERT(m_state == TERM 
            || m_state == INIT);
        StackAllocator::Dealloc(m_stack, m_stacksize);
        m_stack = nullptr;
    } else {
        // 栈不存在，那么函数也不存在
        SYLAR_ASSERT(!m_cb);
        SYLAR_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
}

// 设置当前协程
    // static void SetThis(Fiber* f);

// // 重置协程函数，并重置状态：INIT, TERM
// void reset(std::function<void()> cb);
// // 切换到当前协程执行
// void swapIn();
// // 切换到后台执行
// void swapOut();

// // 返回当前协程
// static Fiber::ptr GetThis();
// // 协程切换到后台，并且设置为Ready状态
// static void YieldToReady();
// // 协程切换到后台，并且设置为Hold状态
// static void YieldToHold();
// // 总协程数
// static uint64_t TotalFibers();

// static MainFunc();

}