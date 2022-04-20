#ifndef __SYLAR_TIMER_H__
#define __SYLAR_TIMER_H__

#include <memory>
#include <set>
#include <vector>
#include "thread.h"

namespace sylar {

class TimerManager;
class Timer: public std::enable_shared_from_this<Timer> {
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;
    bool cancel();
    bool refresh();
    bool reset(uint64_t ms, bool from_now);  // 重新设置执行周期
private:
    Timer(uint64_t ms, std::function<void()> cb
        , bool recurring, TimerManager* manager);
    Timer(uint64_t next);
private:
    bool m_recurring = false;   // 是否循环定时器
    uint64_t m_ms = 0;          // 执行周期
    uint64_t m_next = 0;        // 精确的执行时间戳
    std::function<void()> m_cb;
    TimerManager* m_manager = nullptr;
private:
    struct Comparator {
        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
    };
};

class TimerManager {
friend class Timer;
public:
    typedef RWMutex RWMutexType;

    TimerManager();
    virtual ~TimerManager();

    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb
                        , bool recurring = false);
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb
                        , std::weak_ptr<void> weak_cond
                        , bool recurring = false);
    // 距离最近的下次next_time的时间差
    uint64_t getNextTimer();
    void listExpiredCb(std::vector<std::function<void()>>& cbs);
    bool hasTimer();
protected:
    // 提醒IOManager修改epoll_wait的timeout
    virtual void onTimerInsertedAtFront() = 0;  // 纯虚函数
    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);
private:
    // 时间跨度过大时，进行提醒，作用于listExpiredCb
    bool detectClockRollover(uint64_t now_ms);
private:
    RWMutexType m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    // 保证调用addTimer时仅可能执行一次onTimerInsertedAtFront
    bool m_tickled = false;
    uint64_t m_previousTime = 0;
};

}

#endif