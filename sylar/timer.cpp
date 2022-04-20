#include "timer.h"
#include "util.h"

namespace sylar {
bool Timer::Comparator::operator()(const Timer::ptr& lhs, 
                                   const Timer::ptr& rhs) const {
    if(!lhs && !rhs) {
        return false;
    }
    if(!lhs) {
        return true;
    }
    if(!rhs) {
        return false;
    }
    if(lhs->m_next < rhs->m_next) {
        return true;
    }
    if(rhs->m_next < lhs->m_next) {
        return false;
    }
    // 时间戳相等就对比地址
    return lhs.get() < rhs.get();
}

Timer::Timer(uint64_t ms, std::function<void()> cb, 
             bool recurring, TimerManager* manager) 
:m_recurring(recurring)
,m_ms(ms)
,m_cb(cb)
,m_manager(manager)
{
    m_next = sylar::GetCurrentMS() + m_ms;
}

Timer::Timer(uint64_t next):m_next(next) {

}

bool Timer::cancel() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb)   return false;
    m_cb = nullptr;
    auto it = m_manager->m_timers.find(shared_from_this());
    // 如果it == end()也是true
    if(it != m_manager->m_timers.end())
        m_manager->m_timers.erase(it);
    return true;
}

// 刷新时间，为什么重新加入是因为set排序的原因，如果直接修改元素会产生问题
bool Timer::refresh() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb)  return false;
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) return false;
    m_manager->m_timers.erase(it);
    // 重新加入
    m_next = sylar::GetCurrentMS() + m_ms;
    m_manager->addTimer(shared_from_this(), lock);
    return true;
}

bool Timer::reset(uint64_t ms, bool from_now) {
    if(ms == m_ms && !from_now) {
        // 因为什么也不会变
        return true;
    }
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb)  return false;
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) return false;
    m_manager->m_timers.erase(it);
    // 重新加入
    uint64_t start = 0;
    if(from_now) {
        start = sylar::GetCurrentMS();
    } else {
        start = m_next - m_ms;
    }
    m_ms = ms;
    m_next = start + m_ms;
    m_manager->addTimer(shared_from_this(), lock);
    return true;
}

TimerManager::TimerManager() {
    m_previousTime = sylar::GetCurrentMS();
}

TimerManager::~TimerManager() {

}

Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb
                    , bool recurring) {
    Timer::ptr timer(new Timer(ms, cb, recurring, this));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer, lock);
    return timer;
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
    std::shared_ptr<void> tmp = weak_cond.lock();
    if(tmp) {
        cb();
    }
}

Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb
                    , std::weak_ptr<void> weak_cond
                    , bool recurring) {
    return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
}

uint64_t TimerManager::getNextTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled = false;
    if(m_timers.empty()) {
        return ~0ull;
    }

    const Timer::ptr& next = *m_timers.begin();
    uint64_t now_ms = sylar::GetCurrentMS();
    if(now_ms >= next->m_next) {
        // 过期了
        return 0;
    } else {
        return next->m_next - now_ms;
    }
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs) {
    uint64_t now_ms = sylar::GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty()) {
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);

    bool rollover = detectClockRollover(now_ms);
    if(!rollover && ((*m_timers.begin())->m_next > now_ms)) {
        // 无expired的timer
        return;
    }

    Timer::ptr now_timer(new Timer(now_ms));
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer); // who >= now_timer;
    while(it != m_timers.end() && (*it)->m_next == now_ms) {
        ++it;
    }
    // 插入过期timer
    expired.insert(expired.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired.size());

    for(auto& timer:expired) {
        cbs.push_back(timer->m_cb);
        if(timer->m_recurring) {
            // 允许重复, ps:那些没有m_cb的，其m_recurring=false
            timer->m_next = now_ms + timer->m_ms;
            m_timers.insert(timer);
        } else {
            timer->m_cb = nullptr;
        }
    }
}

bool TimerManager::hasTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}

void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock& lock) {
    auto it = m_timers.insert(val).first;
    bool at_front = (it == m_timers.begin() && !m_tickled);
    if(at_front) m_tickled = true;
    lock.unlock();
    if(at_front) {
        onTimerInsertedAtFront();
    }
}

bool TimerManager::detectClockRollover(uint64_t now_ms) {
    bool rollover = false;
    // 还小于一个小时前
    if(now_ms < m_previousTime &&
       now_ms < (m_previousTime - 60 * 60 * 1000)) {
           rollover = true;
    }
    m_previousTime = now_ms;
    return rollover;
}

}