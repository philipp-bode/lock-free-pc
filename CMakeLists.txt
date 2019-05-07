cmake_minimum_required(VERSION 3.5)
project(lockfreepc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_BUILD_TYPE Release)

option(WITH_STATS "Output stats" ON)

if(WITH_STATS)
    add_definitions(-DWITH_STATS)
endif()

set(SOURCE_FILES main.cpp constraint.hpp constraint.cpp graph.hpp graph.cpp worker.h worker.cpp skeleton.h skeleton.cpp)

message("${CMAKE_MODULES_PATH}")

add_executable(lockfreepc ${SOURCE_FILES})

find_package(GSL REQUIRED)

find_package(Armadillo REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenMP REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})

target_link_libraries(lockfreepc
    pthread
    GSL::gsl
    GSL::gslcblas
    ${ARMADILLO_LIBRARIES}
    OpenMP::OpenMP_CXX
)
