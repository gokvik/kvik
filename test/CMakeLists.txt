cmake_minimum_required(VERSION 3.14)
project(kvik_test)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.0
)
FetchContent_MakeAvailable(Catch2)

# Parse version from git
execute_process(
    COMMAND git describe --all --tags --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

include_directories(
  "../include"
  "../priv_include"
  "include"
)
file(GLOB srcs CONFIGURE_DEPENDS
  "../src/common/*.cpp"
  "../src/linux/*.cpp"
  "src/*.cpp"
  "tests/*.cpp"
)
add_executable(${PROJECT_NAME} ${srcs})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2)
target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE
  "-DGIT_VERSION=\"${GIT_VERSION}\""
)

# Coverage
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  include(CodeCoverage)
  append_coverage_compiler_flags_to_target(${PROJECT_NAME})
  set(GCOVR_ADDITIONAL_ARGS --exclude-unreachable-branches)
  setup_target_for_coverage_gcovr_html(
    NAME ${PROJECT_NAME}_coverage
    EXECUTABLE ${PROJECT_NAME}
    BASE_DIRECTORY ../
    EXCLUDE test
  )
endif()
