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
