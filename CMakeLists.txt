
cmake_minimum_required(VERSION 3.7...3.26)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

set(CMAKE_CXX_STANDARD 20)

project(GrayCattus
    VERSION 1.0
    DESCRIPTION "Cattus API"
    LANGUAGES CXX
)

project(GrayCattusTests
    VERSION 1.0
    DESCRIPTION "Cattus API"
    LANGUAGES CXX
)

add_executable(GrayCattus main.cpp)

add_executable(GrayCattusTests test.cpp)

set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL")
find_package(OpenSSL REQUIRED)
target_link_libraries(GrayCattus PRIVATE OpenSSL::SSL OpenSSL::Crypto OpenSSL::applink)


set(PostgreSQL_ROOT "C:/Program Files/PostgreSQL/15")
find_package(PostgreSQL REQUIRED)
target_link_libraries(GrayCattus PRIVATE PostgreSQL::PostgreSQL)

set(BOOST_ROOT "C:/Program Files/boost")
if (UNIX)
find_package(Boost 1.82.0 REQUIRED COMPONENTS json)
else ()
find_package(Boost 1.82.0 REQUIRED)
endif (UNIX)

target_include_directories(GrayCattus PRIVATE ${Boost_INCLUDE_DIRS})
target_link_directories(GrayCattus PRIVATE ${Boost_LIBRARY_DIRS})
target_link_libraries(GrayCattus PRIVATE ${Boost_LIBRARIES})

target_include_directories(GrayCattus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
