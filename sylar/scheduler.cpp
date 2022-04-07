#include "scheduler.h"
#include "log.h"
#include "macro.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_scheduler_fiber = nullptr;  // 本线程的主协程

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name) 
:m_name(name) {
    SYLAR_ASSERT(threads > 0)
    
    if(use_caller) {
        // 获取当前主线程的主协程
        Fiber::GetThis();
        // 分出一个线程给scheduler
        --threads;

        // 调度器此时再构造所以不会产生第二个调度器
        SYLAR_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this)));
        Thread::SetName(m_name);

        t_scheduler_fiber = m_rootFiber.get();
        m_rootThread = GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1;  // 默认一般线程id是-1表示任意线程
    }
    m_threadCount = threads;
}


Scheduler::~Scheduler() {
    SYLAR_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}


Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}


Fiber* Scheduler::GetMainFiber() {
    return t_scheduler_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if(!m_stopping) {
        // 已经启动了，所以m_stopping=false
        return;
    }
    // 从true改为false，表示启动线程
    m_stopping = false;
    SYLAR_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this),
                           m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();

    // if(m_rootFiber) {
    //     m_rootFiber->swapIn();
    // }
}

void Scheduler::stop() {
    m_autoStop = true;
    // 一个useCaller=true的scheduler线程
    if (m_rootFiber && m_threadCount == 0
        && (m_rootFiber->getState() == Fiber::TERM
        || m_rootFiber->getState() == Fiber::INIT)) {
            SYLAR_LOG_INFO(g_logger) << this << " stopped";
            m_stopping = true;

            if (stopping()) {
                return;
            }
    }
    
    // bool exit_on_this_fiber = false;
    if(m_rootThread != -1) {
        // 调度器所在线程一定是自己
        SYLAR_ASSERT(GetThis() == this);
    } else {
        // other线程
        SYLAR_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();  // 唤醒线程
    }

    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {
        if(!stopping()) {
            // 不可以停止就直接继续运行
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
}

void Scheduler::SetThis() {
    t_scheduler = this;
}

// 主线程或者other线程都会执行
void Scheduler::run() {
    SYLAR_LOG_DEBUG(g_logger) << m_name << " run";
    // set_hook_enable(true);
    SetThis();
    if(sylar::GetThreadId() != m_rootThread) {
        // t_scheduler_fiber原本设置的是m_rootFiber的，所以需要更换
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    // 当调度任务完成后，进行idle_fiber
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    // 载入回调函数的cb_fiber
    Fiber::ptr cb_fiber;

    // 放置合格的任务到ft
    FiberAndThread ft;
    while(true) {
        ft.reset();
        // 有other task存在并且此次没运行
        bool tickle_me = false;
        // 本次线程中的任务被选中
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                // 如果it->threadId == -1代表任意线程
                if(it->threadId != -1 && it->threadId != sylar::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                SYLAR_ASSERT(it->fiber || it->cb);
                // 协程已经运行结束了
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
        }

        if(tickle_me) {
            tickle();
        }
        
        // 上面获得ft
        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT)) {
            ft.fiber->swapIn();
            --m_activeThreadCount;

            if(ft.fiber->getState() == Fiber::READY) {
                // 没执行完成，继续放入，最后还是tickle触发
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                   && ft.fiber->getState() != Fiber::EXCEPT) {
                // 此时的ft.fiber可能是INIT或者EXEC、HOLD
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        } else if(ft.cb) {
            // 如果是cb回调函数
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset(); // 智能指针的引用计数-1
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {
                // 此时cb_fiber可能是EXEC或者INIT、HOLD
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset(); // 智能指针的引用计数-1
            }
        } else {
            if(is_active) {
                // 有活跃的协程
                --m_activeThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM) {
                SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            // 因为是空闲协程所以只会运行和停止
            if(idle_fiber->getState() != Fiber::TERM
            &&idle_fiber->getState() != Fiber::EXCEPT) {
                    idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}

void Scheduler::tickle() {
    SYLAR_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    Mutex::Lock lock(m_mutex);
    return m_autoStop && m_stopping
            && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    SYLAR_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        // 不能停止的话，强行停止
        sylar::Fiber::YieldToHold();
    }
}

void Scheduler::switchTo(pid_t threadId) {
    SYLAR_ASSERT(Scheduler::GetThis() != nullptr);
    if(Scheduler::GetThis() == this) {
        if(threadId == -1 || threadId == sylar::GetThreadId()) {
            // 线程本协程调度器管理的
            return;
        }
    }
    // 把当前协程加入协程调度器中
    schedule(Fiber::GetThis(), threadId);
    // 暂停当前协程
    Fiber::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os) {
    os << "[Scheduler name=" << m_name
       << " size=" << m_threadCount
       << " active_count=" << m_activeThreadCount
       << " idle_count=" << m_idleThreadCount
       << " stopping=" << m_stopping
       << " ]" << std::endl << "    ";
    for(size_t i = 0; i < m_threadIds.size(); ++i) {
        if(i) {
            os << ", ";
        }
        os << m_threadIds[i];
    }
    return os;
}

SchedulerSwitcher::SchedulerSwitcher(Scheduler* target) {
    m_caller = Scheduler::GetThis();
    if(target) {
        target->switchTo();
    }
}

SchedulerSwitcher::~SchedulerSwitcher() {
    if(m_caller) {
        m_caller->switchTo();
    }
}

}

