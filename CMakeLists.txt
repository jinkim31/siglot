cmake_minimum_required(VERSION 3.20)
project(siglot)

set(CMAKE_CXX_STANDARD 20)

option(SIGLOT_WITH_GRAPHVIZ "Use Graphviz" ON)


if(${SIGLOT_WITH_GRAPHVIZ})
    add_compile_definitions(SIGLOT_WITH_GRAPHVIZ)
endif ()

add_library(siglot STATIC
        siglot/object.cpp
        siglot/thread.cpp
        siglot/connection.cpp
        siglot/lookup.cpp
        siglot/observer.cpp
        siglot/timer.cpp)
if(${SIGLOT_WITH_GRAPHVIZ})
target_link_libraries(siglot PRIVATE gvc cgraph)
endif ()

target_include_directories(siglot PUBLIC .)

add_executable(example_consumer_producer example/consumer_producer.cpp)
target_link_libraries(example_consumer_producer PRIVATE siglot)

add_executable(example_copied_emission example/copied_emission.cpp)
target_link_libraries(example_copied_emission PRIVATE siglot)

add_executable(example_moved_emission example/moved_emission.cpp)
target_link_libraries(example_moved_emission PRIVATE siglot)

add_executable(example_service example/service.cpp)
target_link_libraries(example_service PRIVATE siglot)