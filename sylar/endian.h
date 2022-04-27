/**
 * @file endian.h
 * @brief 字节序操作函数(大端/小端)
 */
#ifndef __SYLAR_ENDIAN_H__
#define __SYLAR_ENDIAN_H__

#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>
#include <type_traits>

namespace sylar {

/**
 * @brief 8字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

/**
 * @brief 4字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

/**
 * @brief 2字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

// 这些字节序转换，原字节序的变量第一次转换是一定能转换成功的，但转换回去需要使用原来的转换函数

#if BYTE_ORDER == BIG_ENDIAN
#define SYLAR_BYTE_ORDER SYLAR_BIG_ENDIAN   // 大端
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN   // 小端
#endif

#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 * 目标转大端
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 * 目标转小端
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}

#else

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 * 目标转大端
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 * 目标转小端
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif

}

#endif
