cmake_minimum_required(VERSION 3.16)
project(bfc VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (DEFINED BFC_TARGET_SYSTEM)
    list(GET BFC_TARGET_SYSTEM 0 ISA)
    list(GET BFC_TARGET_SYSTEM 1 ABI)
    list(GET BFC_TARGET_SYSTEM 2 OS_NAME)

    message(STATUS "Building a cross-compiler")
else()
    if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(ISA amd64)
    elseif (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(ISA arm64)
    endif()

    if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
        set(OS_NAME "Linux")
        set(ABI "sysV")
    elseif (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
        set(OS_NAME "macOS")
        if (${ISA} STREQUAL "arm64")
            set(ABI "arm64e")
        else()
            message(FATAL_ERROR "Unsupported ISA/ABI for macOS!")
        endif()
    else()
        message(FATAL_ERROR "Unsupported OS (${CMAKE_HOST_SYSTEM_NAME})!")
    endif()
endif()
message(STATUS "Building a compiler for ${OS_NAME} on architecture ${ISA} (ABI: ${ABI})")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX "isa-specific")
list(APPEND SOURCES
    "${CMAKE_SOURCE_DIR}/src/compilation/backend/isa-specific/${ISA}/${ABI}/${OS_NAME}/buildSpecific.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src/compilation
    src/compilation/backend
    src/compilation/backend/isa-specific
    src/types
    src/utils
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    BFC_VERSION="${PROJECT_VERSION}"
)

include(cmake/CPM.cmake)
CPMAddPackage("gh:prevter/slic@1.0.1")
CPMAddPackage("gh:fmtlib/fmt#12.2.0")
target_link_libraries(${PROJECT_NAME} slic fmt)