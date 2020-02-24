cmake_minimum_required(VERSION 2.8.12)
project(apollo-bridge)

add_definitions("-std=c++17")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
        proto/error_code.proto
        proto/header.proto
        proto/pointcloud.proto
        proto/sensor_image.proto)


add_executable(step1
    step1/step1.cc
    step1/server.h
    step1/server.cc
    step1/client.h
    step1/client.cc
)
target_link_libraries(step1 ${CONAN_LIBS})

add_executable(step2
        step2/step2.cc
        step2/server.h
        step2/server.cc
        step2/client.h
        step2/client.cc
        ${PROTO_HDRS}
        ${PROTO_SRCS}
        )
target_link_libraries(step2 ${CONAN_LIBS})
target_include_directories(step2 PRIVATE ${CMAKE_BINARY_DIR})
