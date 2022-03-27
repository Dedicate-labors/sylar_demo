#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "log.h"
#include <atomic>

namespace sylar {

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id(0);
static std::atomic<uint64_t> s_fiber_count(0);
static thread_local Fiber* t_fiber = nullptr; // 表示目前协程
static thread_local Fiber::ptr t_threadFiber = nullptr; // main协程

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
    // 默认构造main协程
    m_state = EXEC;
    SetThis(this);
    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }
    ++s_fiber_count;
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id=0";
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
    m_ctx.uc_link = &t_threadFiber->m_ctx;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);  // 不会执行
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        SYLAR_ASSERT(m_state == TERM 
            || m_state == INIT
            || m_state == EXCEPT);
        StackAllocator::Dealloc(m_stack, m_stacksize);
        m_stack = nullptr;
    } else {
        // 栈不存在，那么函数也不存在
        SYLAR_ASSERT(!m_cb);
        // 结束状态
        m_state = TERM;
        // 只释放自己管理的fiber
        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id;
}

// 重置协程函数，并重置状态：INIT, TERM
void Fiber::reset(std::function<void()> cb) {
    SYLAR_ASSERT(m_stack);
    SYLAR_ASSERT(m_state == TERM 
            || m_state == INIT
            || m_state == EXCEPT);
    m_cb.swap(cb);

    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = &t_threadFiber->m_ctx;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

// 切换到当前协程执行
void Fiber::swapIn() {
    SetThis(this);
    SYLAR_ASSERT(m_state != EXEC);

    // 此时保存了当前main的上下文到t_threadFiber->m_ctx
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext")
    }
}

// 切换到后台执行
void Fiber::swapOut() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext")
    }
}

// 设置当前协程
void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

// 返回当前协程
Fiber::ptr Fiber::GetThis() {
    if(t_fiber) {
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);  // 默认构造内有SetThis函数，但没m_cb
    SYLAR_ASSERT(t_fiber == main_fiber.get())
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

// 协程切换到后台，并且设置为Ready状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}


// 协程切换到后台，并且设置为Hold状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
}


// 总协程数
uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

// 主体函数，切换m_state
void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();  // 执行m_cb函数，默认构造是没有m_cb的
        cur->m_cb = nullptr;
        cur->m_state = TERM;
        SetThis(t_threadFiber.get());  // 当前协程设置回主协程
    } catch(std::exception& ex) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what();
    } catch(...) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: ";
    }
}


uint64_t Fiber::GetFiberId() {
    if(t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

}