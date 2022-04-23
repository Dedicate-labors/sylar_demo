#include "thread.h"
#include "log.h"
#include "util.h"

namespace sylar {

// 这两个变量是相当于主线程的
static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

Thread* Thread::GetThis() {
    return t_thread;
}

const std::string& Thread::GetName() {
    return t_thread_name;
}

void Thread::SetName(const std::string& name) {
    if(name.empty()) return;

    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

/**
 * @brief Construct a new Thread:: Thread object，初始化成员
 * 
 * @param[in] cb 
 * @param[in] name 
 */
Thread::Thread(std::function<void()> cb, const std::string& name) 
:m_cb(cb)
,m_name(name)
{
    if(name.empty()) {
        m_name = "UNKNOW";
    }
    // On  success, pthread_create() returns 0;
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if(rt) {
        SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
            << " name=" << name;
        throw std::logic_error("pthread_create error");
    }
    // 等待线程创建好
    m_semaphore.wait();
}

Thread::~Thread() {
    if(m_thread) {
        // 如果有用户获取了m_thread，此时detach了线程，那之前的m_thread就回出错
        pthread_detach(m_thread);
    }
}

/**
 * @brief 阻塞线程等待返回，理论应该返回bool值
 * 
 */
void Thread::join() {
    if(m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if(rt) {
            SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
            << " name=" << m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

/**
 * @brief 在Thread构造函数中便开始执行，以对线程id/name等成员进行赋值
 * 
 * @param[in] arg 原始线程对象本身
 * @return void* 
 */
void* Thread::run(void* arg) {
    // 下面只是类型转换
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = GetThreadId();
    // pthread_getname_np()获取名称
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
    std::function<void()> cb;
    cb.swap(thread->m_cb);
    thread->m_semaphore.notify();
    cb();
    return 0;
}

}
