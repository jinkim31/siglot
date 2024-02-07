cmake_minimum_required(VERSION 3.26)
project(ethread2)

set(CMAKE_CXX_STANDARD 17)

add_executable(ethread2 consumer_producer.cpp EObject.cpp
        EThread.h
        ELookup.h
        ELookup.cpp
        EThread.cpp
        EObserver.cpp
        EConnection.h
        EConnection.cpp)

#include_directories(/opt/brew/include)
#link_directories(/opt/brew/lib)
#link_directories(/opt/brew/lib/graphviz)

#find_library(cgraph NAMES libgvc.dylib REQUIRED)

#target_link_libraries(${PROJECT_NAME} ${LIBSODIUM})


add_executable(graph graph.cpp)
#find_library(plugin NAMES graphviz/libgvplugin_dot_layout.dylib REQUIRED)
target_link_libraries(graph PRIVATE gvc cgraph xdot)
