cmake_minimum_required (VERSION 3.0.0)
project(asio)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE asio/asio/include)
