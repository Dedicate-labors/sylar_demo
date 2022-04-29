#include "../sylar/sylar.h"
#include "../sylar/iomanager.h"
#include "../sylar/bytearray.h"
#include "../sylar/address.h"
#include "../sylar/socket.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void server() {
    // 准备addr和socket
    auto addr = sylar::IPAddress::Create("127.0.0.1", 9734);
    SYLAR_ASSERT(addr);
    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    bool rt = sock->bind(addr);
    SYLAR_ASSERT(rt);
    rt = sock->listen();
    SYLAR_ASSERT(rt);
    sylar::ByteArray::ptr ba(new sylar::ByteArray());

    SYLAR_LOG_INFO(g_logger) << "server ready lisiten!";

    while (true)
    {
        ba->clear();
        auto client_sock = sock->accept();
        SYLAR_ASSERT(client_sock);
        // 接受数据
        std::string buffs;
        buffs.resize(4096);
        int rt = client_sock->recv(&buffs[0], buffs.size());
        SYLAR_LOG_INFO(g_logger) << "recv size=" << rt;
        if(rt <= 0) {
            SYLAR_LOG_INFO(g_logger) << "recv fail rt=" << rt;
            return;
        }
        buffs.resize(rt);
        // 二进制反序列化
        ba->writeStringWithoutLength(buffs);
        ba->setPosition(0);
        buffs = ba->readStringF16();
        std::cout << buffs << std::endl;

        ba->clear();
        // 发送数据
        std::string record = "The server receives the message";
        ba->writeStringF16(record);  // 二进制序列化
        ba->setPosition(0);
        record = ba->toString();     // 获取要发送的二进制
        rt = client_sock->send(&record[0], record.size());
        SYLAR_LOG_INFO(g_logger) << "send size=" << rt;
        if(rt <= 0) {
            SYLAR_LOG_INFO(g_logger) << "send fail rt=" << rt;
            return;
        }
        break;
    }
}

int main(int argc, char** argv) {
    sylar::IOManager iom(1,true,"server_iom");
    iom.schedule(server);
    return 0;
}