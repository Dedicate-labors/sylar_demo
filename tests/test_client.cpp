#include "../sylar/sylar.h"
#include "../sylar/iomanager.h"
#include "../sylar/bytearray.h"
#include "../sylar/address.h"
#include "../sylar/socket.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void client() {
    // 准备addr和socket
    auto addr = sylar::IPAddress::Create("127.0.0.1", 9734);
    SYLAR_ASSERT(addr);
    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    if(!sock->connect(addr)) {
        SYLAR_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        SYLAR_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
    }

    sylar::ByteArray::ptr ba(new sylar::ByteArray());
    // 发送数据
    std::string record = "hello world, client";  // 19
    ba->writeStringF16(record);  // 二进制序列化
    ba->setPosition(0);
    record = ba->toString();     // 获取要发送的二进制
    int rt = sock->send(&record[0], record.size());
    // SYLAR_LOG_INFO(g_logger) << "send size=" << rt;
    if(rt <= 0) {
        SYLAR_LOG_INFO(g_logger) << "send fail rt=" << rt;
        return;
    }

    ba->clear();
    // 接受数据
    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());
    // SYLAR_LOG_INFO(g_logger) << "recv size=" << rt;
    if(rt <= 0) {
        SYLAR_LOG_INFO(g_logger) << "recv fail rt=" << rt;
        return;
    }
    buffs.resize(rt);
    // 二进制反序列化
    ba->writeStringWithoutLength(buffs);
    ba->setPosition(0);
    buffs = ba->readStringF16();
    // SYLAR_LOG_INFO(g_logger) << buffs;
}

int main(int argc, char** argv) {
    uint64_t start = sylar::GetCurrentMS();
    {
        sylar::IOManager iom(1,false,"client_iom");
        for(int i = 0; i < 10000; i++) {
            iom.schedule(client);
        }
    }
    uint64_t end = sylar::GetCurrentMS();
    SYLAR_LOG_INFO(g_logger) << "平均耗时（单位秒）：" << ((end-start) * 1.0)/1000;
    return 0;
}