#include "thread.h"
#include "log.h"
#include "util.h"

namespace sylar {

// 这两个变量是相当于主线程的
static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");


Semaphore::Semaphore(uint32_t count) {
    if(sem_init(&m_semaphore, 0, count)) {
        throw std::logic_error("sem init error");
    }
}


Semaphore::~Semaphore() {
    sem_destroy(&m_semaphore);
}


void Semaphore::wait() {
   if(sem_wait(&m_semaphore)) {
        throw std::logic_error("sem_wait error");
   }
}


void Semaphore::notify() {
    if(sem_post(&m_semaphore)) {
        throw std::logic_error("sem_post error");
    }
}


Thread* Thread::GetThis() {
    return t_thread;
}


const std::string& Thread::GetName() {
    return t_thread_name;
}


void Thread::SetName(const std::string& name) {
    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}


// 为什么要绕一圈进行执行？类似装饰器的效果，进行额外处理
// 并且只有这样才能获取到子线程的threadId
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


// 待解决
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


void Thread::join() { // 理论上应该返回bool值
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

}