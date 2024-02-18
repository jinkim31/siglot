cmake_minimum_required(VERSION 3.26)
project(siglot)

set(CMAKE_CXX_STANDARD 17)

add_library(siglot SHARED
        siglot/object.cpp
        siglot/thread.cpp
        siglot/connection.cpp
        siglot/lookup.cpp
        siglot/observer.cpp
        siglot/timer.cpp)
target_link_libraries(siglot PRIVATE gvc cgraph)
target_include_directories(siglot PUBLIC .)

add_executable(example_consumer_producer example/consumer_producer.cpp)
target_link_libraries(example_consumer_producer PRIVATE siglot)

add_executable(example_consumer_producer_copy example/consumer_producer_copy.cpp)
target_link_libraries(example_consumer_producer_copy PRIVATE siglot)

add_executable(example_consumer_producer_move example/consumer_producer_move.cpp)
target_link_libraries(example_consumer_producer_move PRIVATE siglot)

add_executable(example_server_client example/server_client.cpp)
target_link_libraries(example_server_client PRIVATE siglot)

add_executable(test example/test.cpp)
target_link_libraries(test PRIVATE siglot)