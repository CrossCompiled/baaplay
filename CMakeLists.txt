cmake_minimum_required(VERSION 2.8.12)

project(baaplay CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Boost 1.55.0 COMPONENTS filesystem program_options system REQUIRED)

set(SRCS
  src/baaplay.cpp
  src/AlsaPlayer.cpp
)

set(LIBS
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  asound
)

add_executable(baaplay ${SRCS})
target_include_directories(baaplay PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(baaplay PRIVATE ${LIBS})
target_compile_definitions(baaplay PRIVATE BOOST_ASIO_DISABLE_EPOLL)
