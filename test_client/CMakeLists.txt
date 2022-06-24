cmake_minimum_required(VERSION 3.0.0)
project(cartilage_client_test)

source_group("src" FILES main.cpp)

add_executable(${PROJECT_NAME} main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME} PRIVATE cartilage)

