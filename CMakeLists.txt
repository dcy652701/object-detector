cmake_minimum_required(VERSION 3.7)
project(openann)
find_package( OpenCV REQUIRED )

set(CMAKE_BUILD_TYPE          Debug)
set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -fopenmp -pthread -std=c++14")

# Edge Detector
add_executable(
  edge-detector
  src/edge-detector.cpp
)

target_link_libraries( edge-detector ${OpenCV_LIBS} )
