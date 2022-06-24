cmake_minimum_required (VERSION 3.0.0)
project(asio)

set(SOURCE_FILES asio/asio/src/asio.cpp)
source_group("src" FILES ${SOURCE_FILES})

add_library(asio ${SOURCE_FILES})
