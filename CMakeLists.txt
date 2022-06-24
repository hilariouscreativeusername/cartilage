cmake_minimum_required(VERSION 3.0.0)
project(cartilage)

set(SOURCE_FILES include/cartilage.hpp src/cartilage.cpp)
source_group("src" FILES ${SOURCE_FILES})

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} INTERFACE include)

add_subdirectory(test_server)
add_subdirectory(test_client)

