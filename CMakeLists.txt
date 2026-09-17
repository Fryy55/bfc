cmake_minimum_required(VERSION 3.16)
project(bfc VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(OS_NAME "Linux")
    set(ISA "amd64")
    set(ABI "sysV")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(OS_NAME "macOS")
    set(ISA "arm64")
    set(ABI "sysV")
else()
    message(FATAL_ERROR "Unsupported ABI/ISA!")
endif()
message(STATUS "Building on ${OS_NAME} for architecture ${ISA} (ABI ${ABI})")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
foreach(SOURCE ${SOURCES})
    if ("${SOURCE}" MATCHES "isa-specific")
        list(REMOVE_ITEM SOURCES "${SOURCE}")
    endif()
endforeach()
list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/compilation/backend/isa-specific/${ISA}/${ABI}/buildSpecific.cpp")
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