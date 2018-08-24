cmake_minimum_required(VERSION 3.12)
project(OpenMPI)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_COMPILER "/usr/bin/mpic++")
set(CMAKE_C_COMPILER "/usr/bin/mpicc")

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(HelloWorldMPI main.cpp)
target_link_libraries(HelloWorldMPI ${MPI_C_LIBRARIES})
