
cmake_minimum_required(VERSION 3.7...3.26)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(GrayCattus
    VERSION 1.0
    DESCRIPTION "Cattus API"
    LANGUAGES CXX
)

add_executable(GrayCattus main.cpp)

target_include_directories(GrayCattus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} /usr/include/openssl /usr/include/postgresql)
target_link_libraries(GrayCattus PRIVATE crypto ssl)
