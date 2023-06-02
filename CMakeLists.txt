cmake_minimum_required(VERSION 3.0.0)
project(Modem VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RtAudio REQUIRED IMPORTED_TARGET rtaudio)

SET(CMAKE_BUILD_TYPE Release)
add_compile_options(-Ofast)

if(WIN32)
    set(SERIAL_IMPL serial_windows.cpp)
else()
    set(SERIAL_IMPL serial_linux.cpp)
endif()

add_executable(Modem main.cpp uart.cpp v21.cpp ${SERIAL_IMPL})

target_link_libraries(Modem PkgConfig::RtAudio Threads::Threads)

option(GRADER "Modem tests (grader)" OFF)
if(GRADER)
    add_subdirectory(grader)
endif()
