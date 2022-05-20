#ifndef __SYLAR_IOMANAGER_H__
#define __SYLAR_IOMANAGER_H__

#include "scheduler.h"
#include "timer.h"
#include <sys/epoll.h>

namespace sylar {

class IOManager: public Scheduler,public TimerManager {
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;
    // fd的status
    enum Event {
        NONE = 0x0,
        READ = EPOLLIN,
        WRITE = EPOLLOUT
    };
private:
    struct FdContext {
        typedef Mutex MutexType;
        struct EventContext {
            Scheduler* scheduler = nullptr;  //事件执行的scheduler
            Fiber::ptr fiber;                //事件协程
            std::function<void()> cb;        //事件的回调函数
        };

        EventContext& getContext(Event event);
        // reset传入的ctx，使用时先getContext获取到执行事件
        void resetContext(EventContext& ctx);
        void triggerEvent(Event event);

        EventContext read;      //读事件
        EventContext write;     //写事件
        int fd = 0;                 //事件关联的句柄
        Event events = NONE;  //已注册的事件
        MutexType mutex;
    };
public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name="");
    ~IOManager();

    // 0 success, -1 error
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);     // 事件删除
    bool cancelEvent(int fd, Event event);  // 取消事件和执行条件并强制触发
    bool cancelAll(int fd);                 // 取消句柄上的所有事件

    static IOManager* GetThis();            // 查看和对比数据的(只读)，不能delete
protected:
    // 提醒有协程任务
    void tickle() override;
    // 表示是否能停止
    bool stopping() override;
    // 理解为自定义epoll_wait
    void idle() override;
    void onTimerInsertedAtFront() override;
    void contextResize(size_t size);
    bool stopping(uint64_t& timeout);
private:
    // epoll 文件句柄
    int m_epfd = 0;
    // pipe 文件句柄
    int m_tickleFds[2];
    // 当前等待执行的事件数量
    std::atomic<size_t> m_pendingEventCount = {0};
    RWMutexType m_mutex;
    std::vector<FdContext*> m_fdContexts; 
};

}

#endif