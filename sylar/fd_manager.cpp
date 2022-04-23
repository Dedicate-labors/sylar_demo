#include "fd_manager.h"
#include "hook.h"
#include <sys/types.h>
#include <sys/stat.h>

namespace sylar {

FdCtx::FdCtx(int fd)
:m_isInit(false)
,m_isSocket(false)
,m_sysNonblock(false)
,m_userNonblock(false)
,m_isClosed(false)
,m_fd(fd)
,m_recvTimeout(-1)
,m_sendTimeout(-1)
{
    init();
}

FdCtx::~FdCtx() {
}

bool FdCtx::init() {
    if(m_isInit) {
        return true;
    }
    m_recvTimeout = -1;
    m_sendTimeout = -1;

    struct stat fd_stat;
    // 判断句柄是否打开
    if(-1 == fstat(m_fd, &fd_stat)) {
        // m_fd没打开
        m_isInit = false;
        m_isSocket = false;
    } else {
        // m_fd打开
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    m_sysNonblock = false;
    if(m_isSocket) {
        int flags = fcntl_f(m_fd, F_GETFL, 0);
        if(!(flags & O_NONBLOCK)) {
            // 如果flags不是O_NONBLOCK
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_sysNonblock = true;
    }

    m_userNonblock = false;
    m_isClosed = false;
    return m_isInit;
}

void FdCtx::setTimeout(int type, uint64_t v) {
    if(type == SO_RCVTIMEO) {
        m_recvTimeout = v;
    } else {
        m_sendTimeout = v;
    }
}

uint64_t FdCtx::getTimeout(int type) {
    if(type == SO_RCVTIMEO) {
        return m_recvTimeout;
    }
    return m_sendTimeout;
}

FdManager::FdManager() {
    m_datas.resize(64);
}

FdCtx::ptr FdManager::get(int fd, bool auto_create) {
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        if(auto_create == false) {
            return nullptr;
        }
        m_datas.resize(fd * 1.5);
    } else {
        if(m_datas[fd] || !auto_create) {
            return m_datas[fd];
        }
    }
    lock.unlock();

    //size() > fd 下auto_create == true && !m_datas[fd] 情况
    RWMutexType::WriteLock lock2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    m_datas[fd] = ctx;
    return ctx;
}

void FdManager::del(int fd) {
    RWMutexType::WriteLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        return;
    }
    m_datas[fd].reset();
}

}