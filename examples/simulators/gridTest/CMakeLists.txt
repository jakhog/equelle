cmake_minimum_required(VERSION 2.8)
find_package(Equelle REQUIRED)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include_directories( ${EQUELLE_INCLUDE_DIRS} "../../../../build/backends/cuda" "../../../backends/cuda/cuda_include/" "../../../backends/cuda/include" "/usr/local/cuda-5.5/include" "../../../../build/backends/serial"  "../../../../build/backends/serial/Headers" "../../../../build/backends/serial/src" )

add_executable(gridTest gridTest.cpp)
target_link_libraries( gridTest ${EQUELLE_LIBRARIES}  )

add_executable(serial gridTestCPU.cpp)
target_link_libraries( serial ${EQUELLE_LIBRARIES} )

# If cmake can't find equelle package, run:
# cmake -DEquelle_DIR=/home/havahol/equelle/build/
