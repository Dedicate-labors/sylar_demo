cmake_minimum_required(VERSION 3.0)
project(sylar)

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/utils.cmake)  # 把__file__宏定义为相对路径

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

# include_directories(.)
# include_directories(/apps/sylar/include)
# link_directories(/apps/sylar/lib)
# link_directories(/apps/sylar/lib64)

set(LIB_SRC
    sylar/log.cpp
    sylar/util.cpp
    sylar/config.cpp
    sylar/thread.cpp
    sylar/mutex.cpp
    sylar/fiber.cpp
    sylar/scheduler.cpp
    sylar/timer.cpp
    sylar/iomanager.cpp
    sylar/hook.cpp
    sylar/fd_manager.cpp
    sylar/address.cpp
    sylar/socket.cpp
    sylar/bytearray.cpp
    )

add_library(sylar SHARED ${LIB_SRC})  # 生成动态库
# force_redefine_file_macro_for_sources(sylar)
set(LIB_LIB
    sylar
    pthread
    yaml-cpp
    dl
    ssl
    )

add_executable(test_log ${CMAKE_SOURCE_DIR}/tests/test.cpp)  # 生成可执行文件test_log
add_dependencies(test_log sylar)
# force_redefine_file_macro_for_sources(test_log)
target_link_libraries(test_log ${LIB_LIB})  # 连接动态库

add_executable(test_config tests/test_config.cpp)  # 生成可执行文件test_config
add_dependencies(test_config sylar)
# force_redefine_file_macro_for_sources(test_config)
target_link_libraries(test_config ${LIB_LIB})  # 连接动态库

add_executable(test_thread tests/test_thread.cpp)  # 生成可执行文件test_thread
add_dependencies(test_thread sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_thread ${LIB_LIB})  # 连接动态库

add_executable(test_util tests/test_util.cpp)  # 生成可执行文件test_util
add_dependencies(test_util sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_util ${LIB_LIB})  # 连接动态库

add_executable(test_fiber tests/test_fiber.cpp)  # 生成可执行文件test_fiber
add_dependencies(test_fiber sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_fiber ${LIB_LIB})  # 连接动态库

add_executable(test_scheduler tests/test_scheduler.cpp)  # 生成可执行文件test_scheduler
add_dependencies(test_scheduler sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_scheduler ${LIB_LIB})  # 连接动态库

add_executable(test_iomanager tests/test_iomanager.cpp)  # 生成可执行文件test_iomanager
add_dependencies(test_iomanager sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_iomanager ${LIB_LIB})  # 连接动态库

add_executable(test_hook tests/test_hook.cpp)  # test_hook
add_dependencies(test_hook sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_hook ${LIB_LIB})  # 连接动态库

add_executable(test_address tests/test_address.cpp)  # test_address
add_dependencies(test_address sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_address ${LIB_LIB})  # 连接动态库

add_executable(test_socket tests/test_socket.cpp)  # test_socket
add_dependencies(test_socket sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_socket ${LIB_LIB})  # 连接动态库

add_executable(test_bytearray tests/test_bytearray.cpp)  # test_bytearray
add_dependencies(test_bytearray sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_bytearray ${LIB_LIB})  # 连接动态库

add_executable(test_client tests/test_client.cpp)  # test_client
add_dependencies(test_client sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_client ${LIB_LIB})  # 连接动态库

add_executable(test_server tests/test_server.cpp)  # test_server
add_dependencies(test_server sylar)
# force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_server ${LIB_LIB})  # 连接动态库

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)