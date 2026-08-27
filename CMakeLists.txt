cmake_minimum_required(VERSION 3.16)
project(bfc VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src/utils
    src/compilation
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    BFC_VERSION="${PROJECT_VERSION}"
)

include(cmake/CPM.cmake)
CPMAddPackage("gh:prevter/slic@1.0.1")
target_link_libraries(${PROJECT_NAME} slic)