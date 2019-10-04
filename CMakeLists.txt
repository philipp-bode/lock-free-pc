cmake_minimum_required(VERSION 3.5)
project(lockfreepc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_BUILD_TYPE Release)

find_package( PkgConfig )


option(WITH_STATS "Output stats" ON)

if(WITH_STATS)
    add_definitions(-DWITH_STATS)
endif()

set(SOURCE_FILES constraint.hpp constraint.cpp graph.hpp graph.cpp worker.h worker.cpp skeleton.h skeleton.cpp)

add_executable(lock-free-pc main.cpp ${SOURCE_FILES})

find_package(GSL REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/")

find_package(Boost REQUIRED)
find_package(Armadillo REQUIRED)
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

include_directories(${LAPACK_INCLUDE_DIR})
include_directories(${BLAS_INCLUDE_DIR})
include_directories(${ARMADILLO_INCLUDE_DIRS})
target_link_libraries(lock-free-pc
    pthread
    GSL::gsl
    GSL::gslcblas
    ${ARMADILLO_LIBRARIES}
)

add_subdirectory(pybind11)
pybind11_add_module(lockfreepc _lockfreepc.cpp ${SOURCE_FILES})

target_link_libraries(lockfreepc PRIVATE
    ${ARMADILLO_LIBRARIES}
    ${LAPACK_LIBRARIES}
    ${BLAS_LIBRARIES}
    pthread
    GSL::gsl
    GSL::gslcblas
)
