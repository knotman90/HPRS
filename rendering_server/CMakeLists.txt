cmake_minimum_required(VERSION 3.1)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_PROJECT_DIR}/cmake/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Hbase/gen-cpp)
include_directories("../utility/")
include_directories("../model/")


file(GLOB SRC_utility
    "../utility/*.h"
    "../utility/*.cpp"
)


file(GLOB SRC_model
    "../model/*.h"
    "../model/*.cpp"
)

file(GLOB SRC_server
    source/*.cxx
    source/*.cpp
  )

#ice_t hbase ----------------------------------
find_package(MPI REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_library(IceT REQUIRED)
find_library (GLM REQUIRED)
find_package(Boost REQUIRED system serialization)
include_directories(${MPI_INCLUDE_PATH}) #mpi
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR}, ${GLM_INCLUDE_DIRS})
#icet
include_directories(${ICET_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
link_directories(${ICET_LIBRARY_DIRS})

#bprinter for printing iceT stats
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/bprinter)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/bprinter)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bprinter/include/)


add_executable(renderer_server ${SRC_server} ${SRC_utility} ${SRC_model})
if(MPI_COMPILE_FLAGS)
  set_target_properties(renderer_server PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(renderer_server PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

set_property(TARGET renderer_server PROPERTY CXX_STANDARD 14)
target_link_libraries(renderer_server  ${Boost_SYSTEM_LIBRARY} ${Boost_SERIALIZATION_LIBRARY} pthread) #link mpi glut opengl
target_link_libraries(renderer_server ${OPENGL_LIBRARIES}  ${GLUT_glut_LIBRARY} ${MPI_LIBRARIES} ${MPI_EXTRA_LIBRARY} ${ICET_CORE_LIBS} ${ICET_MPI_LIBS} IceTGL bprinter ) #iceT
