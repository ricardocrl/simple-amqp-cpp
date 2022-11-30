cmake_minimum_required (VERSION 2.6)

project (SimpleAMQPCPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Add AMQP-CPP lib
option(AMQP-CPP_LINUX_TCP "Build linux sockets implementation." ON)
add_subdirectory(external/AMQP-CPP)

# Find Boost libs
FIND_PACKAGE(Boost REQUIRED)

set(INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*
)

add_library(simple-amqpcpp ${SOURCE_FILES})

target_include_directories(simple-amqpcpp PUBLIC ${INCLUDE_DIR})
target_include_directories(simple-amqpcpp SYSTEM PUBLIC external/AMQP-CPP/include)

target_link_options(simple-amqpcpp
    PUBLIC -pthread
)
target_link_libraries(simple-amqpcpp
    ${Boost_LIBRARIES}
    amqpcpp
)

# Example application
add_executable(example example.cpp)
target_link_libraries(example simple-amqpcpp)
