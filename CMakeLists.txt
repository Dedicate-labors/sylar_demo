cmake_minimum_required(VERSION 3.0)
project(sylar)

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# include(cmake/utils.cmake)

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
    )

add_library(sylar SHARED ${LIB_SRC})  # 生成动态库
find_library(YAMLCPP yaml-cpp)

add_executable(test_log ${CMAKE_SOURCE_DIR}/tests/tests.cpp)  # 生成可执行文件test
add_dependencies(test_log sylar)
target_link_libraries(test_log sylar ${YAMLCPP})  # 连接动态库

add_executable(test_config ${CMAKE_SOURCE_DIR}/tests/test_config.cpp)  # 生成可执行文件test
add_dependencies(test_config sylar)
target_link_libraries(test_config sylar ${YAMLCPP})  # 连接动态库

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)