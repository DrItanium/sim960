cmake_minimum_required(VERSION 3.10)

project(i960 VERSION 2.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(core960
        CoreTypes.h
        ArithmeticControls.h
        ArithmeticControls.cpp
        Core.cpp
        Core.h
        DependentFalse.h
        PreviousFramePointer.h
        ProcessControls.h
        TraceControls.h
        Register.h)



add_subdirectory(tests)