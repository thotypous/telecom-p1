cmake_minimum_required(VERSION 3.0.0)
project(Modem VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Modem main.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RtAudio REQUIRED IMPORTED_TARGET rtaudio)

target_link_libraries(Modem PkgConfig::RtAudio)
