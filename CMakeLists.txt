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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/model/)

file(GLOB SRC_HBASE_FILES ${DIR_GEN}/Hbase_types.cpp ${DIR_GEN}/Hbase_constants.cpp ${DIR_GEN}/Hbase.cpp)


file(GLOB SRC_utility
    "utility/*.h"
    "utility/*.cpp"
)


file(GLOB SRC_model
    "model/*.h"
    "model/*.cpp"
)

file(GLOB SRC_thrift
    "thrift/*.h"
    "thrift/*.cpp"
)

find_package( Boost REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )


#testclient hbase
add_executable(HPRS  testclient.cpp ${SRC_HBASE_FILES} ${SRC_utility} ${SRC_model})

set_property(TARGET HPRS PROPERTY CXX_STANDARD 14)
target_link_libraries(HPRS ${Boost_LIBS} ${THRIFT_LIBRARIES})

#ice_t hbase ----------------------------------
find_package(MPI REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package (IceT REQUIRED)
include_directories(${MPI_INCLUDE_PATH}) #mpi

include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR})
#icet
include_directories(${ICET_INCLUDE_DIRS})
link_directories(/home/knotman/git/IceT/lib)

add_executable(HPRS_iceT  ice_tHbase.cpp ${SRC_HBASE_FILES} ${SRC_utility} ${SRC_model})
set_property(TARGET HPRS_iceT PROPERTY CXX_STANDARD 14)
target_link_libraries(HPRS_iceT ${Boost_LIBS} ${THRIFT_LIBRARIES})
target_link_libraries(HPRS_iceT  ${OPENGL_LIBRARIES} ${GLUT_glut_LIBRARY}) #link mpi glut opengl
target_link_libraries(HPRS_iceT ${MPI_LIBRARIES} ${MPI_EXTRA_LIBRARY} ${ICET_CORE_LIBS} ${ICET_MPI_LIBS} IceTGL  ) #iceT

