cmake_minimum_required(VERSION 3.17)
project(sim960)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(.)
add_library(core960
        ArithmeticControls.cpp
        ArithmeticControls.h
        Core.cpp
        Core.h
        CoreTypes.h
        DependentFalse.h
        PreviousFramePointer.h
        ProcessControls.h
        Register.h
        TraceControls.h
        Disassembly.h
        OpcodesRaw.h
        GenericNumber.h
        ProcessorMappingConfiguration.cpp
        ProcessorMappingConfiguration.h
        ProcessorAddress.h
        TraceControls.cpp
        InterfaceUnit.h
        InternalPeripheralUnit.h
        BusInterfaceUnit.h
        InstructionFormats.cpp
        InstructionFormats.h)

add_subdirectory(cmd)