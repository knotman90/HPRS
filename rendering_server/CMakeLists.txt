cmake_minimum_required(VERSION 3.1)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_PROJECT_DIR}/utility/)
include_directories(${CMAKE_PROJECT_DIR}/model/)


file(GLOB SRC_utility
    "utility/*.h"
    "utility/*.cpp"
)


file(GLOB SRC_model
    "${CMAKE_PROJECT_DIR}/model/*.h"
    "${CMAKE_PROJECT_DIR}/model/*.cpp"
)

file(GLOB SRC_server
    source/*.cxx
    source/*.cpp
  )

#ice_t hbase ----------------------------------
find_package(MPI REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package (IceT REQUIRED)
find_library (GLM REQUIRED)
include_directories(${MPI_INCLUDE_PATH}) #mpi
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR})
#icet
include_directories(${ICET_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
link_directories(${ICET_LIBRARY_DIRS})

add_executable(renderer_server ${SRC_server} ${SRC_utility} ${SRC_model})
set_property(TARGET renderer_server PROPERTY CXX_STANDARD 14)
target_link_libraries(renderer_server  ${OPENGL_LIBRARIES} ${GLUT_glut_LIBRARY}) #link mpi glut opengl
target_link_libraries(renderer_server ${MPI_LIBRARIES} ${MPI_EXTRA_LIBRARY} ${ICET_CORE_LIBS} ${ICET_MPI_LIBS} IceTGL  ) #iceT
