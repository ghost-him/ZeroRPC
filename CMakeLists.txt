cmake_minimum_required(VERSION 3.25)
project(ZeroRPC CXX)

# 启用C++20标准,这是使用模块所必需的
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Common/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Common/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Maintain/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RPC/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RPC/*.hpp"
)
file(GLOB_RECURSE INC
        "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Common/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Maintain/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RPC/*.h"
)

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
    add_executable(unitTest ${SRC} ${INC} ${SRC_TEST})

    target_link_libraries(unitTest gtest gtest_main brotlienc brotlidec uuid)

    include(GoogleTest)
    gtest_discover_tests(unitTest)

else()
    # 添加可执行文件
    add_executable(ZeroRPC ${SRC} ${INC})
    target_link_libraries(unitTest brotlienc brotlidec uuid)

endif ()


