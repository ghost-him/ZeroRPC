cmake_minimum_required(VERSION 3.25)
project(MyProject CXX)

# 启用C++20标准,这是使用模块所必需的
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_CORE "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp")
file(GLOB_RECURSE SRC_COMMON "${CMAKE_CURRENT_SOURCE_DIR}/Common/*.cpp")
file(GLOB_RECURSE SRC_COMMON "${CMAKE_CURRENT_SOURCE_DIR}/Common/*.hpp")

include_directories(Common)
include_directories(Core)

set(ENABLE_UNIT_TESTS ON CACHE INTERNAL "Enable unit tests")
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")
if(ENABLE_UNIT_TESTS)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()

    file(GLOB_RECURSE SRC_TEST "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.cpp")
    add_executable(unitTest ${SRC_CORE} ${SRC_TEST})

    target_link_libraries(unitTest gtest gtest_main brotlienc brotlidec)

    include(GoogleTest)
    gtest_discover_tests(unitTest)

else()
    # 添加可执行文件
    add_executable(ZeroRPC ${SRC_CORE} ${SRC_COMMON})


endif ()


