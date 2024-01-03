cmake_minimum_required(VERSION 3.26)
project(ethread2)

set(CMAKE_CXX_STANDARD 20)

add_executable(ethread2 main.cpp EObject.cpp
        EThread.h
        ELookup.h
        ELookup.cpp
        EThread.cpp)
