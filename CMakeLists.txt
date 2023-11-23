cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(trace_stream_loader VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(SDE_LIB_PATH $ENV{SDE_INSTALL}/lib)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

include_directories($ENV{SDE_INSTALL}/include/)

find_package(Threads REQUIRED)

find_library(TOFINOPDFIXED_LIBRARY libtofinopdfixed_thrift.so PATHS ${SDE_LIB_PATH})
find_library(BF_SWITCHD_LIBRARY libbf_switchd_lib.so PATHS ${SDE_LIB_PATH})
find_library(DRIVER_LIBRARY libdriver.so PATHS ${SDE_LIB_PATH})
find_library(BFSYS_LIBRARY libbfsys.so PATHS ${SDE_LIB_PATH})
find_library(BFUTILS_LIBRARY libbfutils.so PATHS ${SDE_LIB_PATH})

add_executable(controller src/controller.cc)

target_link_libraries(controller Threads::Threads)

target_link_libraries(controller
    ${DRIVER_LIBRARY} ${BFUTILS_LIBRARY})