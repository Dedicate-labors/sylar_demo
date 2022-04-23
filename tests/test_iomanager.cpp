#include "../sylar/sylar.h"
#include "../sylar/iomanager.h"
#include <arpa/inet.h>
#include <fcntl.h>


sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int sock_fd = 0;

void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "test_fiber  sock=" << sock_fd;
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock_fd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "39.156.66.14", &addr.sin_addr.s_addr);

    if(!connect(sock_fd, (const sockaddr *)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        // connect返回-1，错误是EINPROGRESS代表连接正在进行

        // read 无法单独执行  可能就是对面根本不回复
        SYLAR_LOG_INFO(g_logger) << "add event errno=" << " " << strerror(errno);
        sylar::IOManager::GetThis()->addEvent(sock_fd, sylar::IOManager::READ, [](){
            SYLAR_LOG_INFO(g_logger) << "read callback";
        });

        // 单独执行write可以完全关闭退出，但无法执行read，需要关闭触发read event
        sylar::IOManager::GetThis()->addEvent(sock_fd, sylar::IOManager::WRITE, [](){
            SYLAR_LOG_INFO(g_logger) << "write callback";
            // 怕不是这里执行的READ事件
            sylar::IOManager::GetThis()->cancelEvent(sock_fd, sylar::IOManager::READ);
            close(sock_fd);
        });
    } else {
        SYLAR_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}

void test1() {
    sylar::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}

sylar::Timer::ptr s_timer;
void test_timer() {
    sylar::IOManager iom(2, false);
    s_timer = iom.addTimer(1000, []() {
        static int i = 0;
        SYLAR_LOG_INFO(g_logger) << "hello timer i = " << i;
        if(++i == 3) {
            s_timer->reset(2000, true);
            // s_timer->cancel();
        }
    }, true);
}

int main(int argc, char **argv) {
    // test1();
    test_timer();
    return 0;
}