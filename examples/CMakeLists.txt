cmake_minimum_required(VERSION 3.1)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

find_package(Boost REQUIRED system serialization)

file(GLOB SRC_SERVER
    "simple_server.cpp"
)
file(GLOB SRC_CLIENT
    "simple_client.cpp"
    "connection.hpp"
)

include_directories( ${Boost_INCLUDE_DIRS} )
include_directories("../model/")
include_directories("../utility/")


add_executable(simple_client ${SRC_CLIENT} )
set_property(TARGET simple_client PROPERTY CXX_STANDARD 11)
target_link_libraries(simple_client ${Boost_SYSTEM_LIBRARY}   ${Boost_SERIALIZATION_LIBRARY} pthread )
