cmake_minimum_required(VERSION 3.0.0)
project(cartilage)

set(SOURCE_FILES include/cartilage.hpp src/cartilage.cpp include/asio_wrapper.hpp include/client.hpp include/connection.hpp include/logger.hpp include/message.hpp include/server.hpp include/tsqueue.hpp)
source_group("src" FILES ${SOURCE_FILES})

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(test_server)
add_subdirectory(test_client)

add_subdirectory(depend/asio)
target_link_libraries(${PROJECT_NAME} PUBLIC asio)

