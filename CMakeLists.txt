cmake_minimum_required(VERSION 3.1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thrift-gen)

include( cmake/FindThrift.cmake)
thrift_gen_cpp(${CMAKE_CURRENT_SOURCE_DIR}/thrift/hbase_thrift/Hbase.thrift SCR_GEN_HBASE_THRIFT DIR_GEN)
#THRIFT FILE GENERATED
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories( ${THRIFT_INCLUDE_DIR})
include_directories( ${DIR_GEN})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utility/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thrift/)


file(GLOB SRC_HBASE_FILES ${DIR_GEN}/Hbase_types.cpp ${DIR_GEN}/Hbase_constants.cpp ${DIR_GEN}/Hbase.cpp)


file(GLOB SRC_utility
    "utility/*.h"
    "utility/*.cpp"
)

file(GLOB SRC_thrift
    "thrift/*.h"
    "thrift/*.cpp"
)

find_package( Boost REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )


add_executable(HPRS  testclient.cpp ${SRC_HBASE_FILES} ${SRC_utility})
target_link_libraries(HPRS ${Boost_LIBS} ${THRIFT_LIBRARIES})