cmake_minimum_required(VERSION 3.0.0)
project(Modem VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("-ffast-math" "-fno-finite-math-only" "-fsigned-zeros" "-fno-associative-math")

add_executable(Modem main.cpp uart.cpp v21.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RtAudio REQUIRED IMPORTED_TARGET rtaudio)

target_link_libraries(Modem PkgConfig::RtAudio)
