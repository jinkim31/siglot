cmake_minimum_required(VERSION 3.26)
project(siglot)

set(CMAKE_CXX_STANDARD 17)

option(SIGLOT_WITH_GRAPHVIZ "Use Graphviz" ON)

MESSAGE("GraphViz?:")
message(${SIGLOT_WITH_GRAPHVIZ})

if(${SIGLOT_WITH_GRAPHVIZ})
    message("ADDING GVIZ DEFINE")
    add_compile_definitions(SIGLOT_WITH_GRAPHVIZ)
endif ()

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

add_executable(example_copied_emission example/copied_emission.cpp)
target_link_libraries(example_copied_emission PRIVATE siglot)

add_executable(example_moved_emission example/moved_emission.cpp)
target_link_libraries(example_moved_emission PRIVATE siglot)

add_executable(example_service example/service.cpp)
target_link_libraries(example_service PRIVATE siglot)