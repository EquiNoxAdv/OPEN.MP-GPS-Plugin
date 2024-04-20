cmake_minimum_required(VERSION 3.10)

project(gps-omp)

set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

include(AddSAMPPlugin)

add_subdirectory(lib/omp-sdk)

include_directories(
  lib/pawn/source
  lib/pawn/source/linux
)

add_samp_plugin(${PROJECT_NAME}

  plugin.def

  src/main.h
  src/common.h
  src/connection.h
  src/container.h
  src/file.h
  src/node.h
  src/path.h
  src/pathfinder.h
  src/connection.cpp
  src/container.cpp
  src/file.cpp
  src/node.cpp
  src/path.cpp
  src/pathfinder.cpp
  src/main.cpp
)

add_definitions(
    -DHAVE_STDINT_H=1
    -DPAWN_CELL_SIZE=32
)

target_include_directories(${PROJECT_NAME} PRIVATE lib lib/pawn/source/amx)
target_link_libraries(${PROJECT_NAME} PRIVATE OMP-SDK)
