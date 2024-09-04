cmake_minimum_required(VERSION 3.14)
project(kvik_test)

Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.0
)
FetchContent_MakeAvailable(Catch2)

include_directories(
  "../include"
  "../priv_include"
)
file(GLOB srcs CONFIGURE_DEPENDS
  "../src/common/*.cpp"
  "../src/testing/*.cpp"
  "tests/*.cpp"
)
add_executable(kvik_test ${srcs})

set_property(TARGET kvik_test PROPERTY CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
target_link_libraries(kvik_test PRIVATE Threads::Threads)
target_link_libraries(kvik_test PRIVATE Catch2::Catch2WithMain)
