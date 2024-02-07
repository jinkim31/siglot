cmake_minimum_required(VERSION 3.26)
project(siglot)

set(CMAKE_CXX_STANDARD 17)

add_library(siglot SHARED
        siglot/Object.cpp
        siglot/Thread.cpp
        siglot/Connection.cpp
        siglot/Lookup.cpp
        siglot/Observer.cpp)
target_link_libraries(siglot PRIVATE gvc cgraph)
target_include_directories(siglot PUBLIC .)

add_executable(example_consumer_producer example/consumer_producer.cpp)
target_link_libraries(example_consumer_producer PRIVATE siglot)