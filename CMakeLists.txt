cmake_minimum_required(VERSION 3.20)

message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

# Enable Hot Reload for MSVC
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if (MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

project("LunchDeliveryAlns")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /arch:AVX2 /fp:fast")
else()
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -ffast-math")
endif()

enable_testing()

find_package(TBB CONFIG REQUIRED)
find_package(highs CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE PROBLEM_SOURCES src/problem/*.cpp)
file(GLOB_RECURSE HEURISTIC_SOURCES src/heuristic/*.cpp)
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

add_library(LunchDeliveryAlns_lib ${PROBLEM_SOURCES} ${HEURISTIC_SOURCES})
add_executable(LunchDeliveryAlns main.cpp)
add_executable(LunchDeliveryAlns_tests ${TEST_SOURCES})

# Main executable
target_link_libraries(LunchDeliveryAlns PRIVATE LunchDeliveryAlns_lib)
target_link_libraries(LunchDeliveryAlns PRIVATE TBB::tbb)
target_link_libraries(LunchDeliveryAlns PRIVATE highs::highs)
target_link_libraries(LunchDeliveryAlns PRIVATE fmt::fmt)

# Shared lib
target_link_libraries(LunchDeliveryAlns_lib PRIVATE TBB::tbb)
target_link_libraries(LunchDeliveryAlns_lib PRIVATE highs::highs)
target_link_libraries(LunchDeliveryAlns_lib PRIVATE fmt::fmt)

# Tests
target_link_libraries(LunchDeliveryAlns_tests PRIVATE LunchDeliveryAlns_lib)
target_link_libraries(LunchDeliveryAlns_tests PRIVATE GTest::gtest_main)
target_link_libraries(LunchDeliveryAlns_tests PRIVATE fmt::fmt)

# IPO/LTO
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL OFF)

# Copy LKH.exe to output directory so it's findable at runtime
# add_custom_command(TARGET LunchDeliveryAlns POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             ${CMAKE_SOURCE_DIR}/third_party/lkh/LKH.exe
#             $<TARGET_FILE_DIR:LunchDeliveryAlns>)

include(GoogleTest)
gtest_discover_tests(LunchDeliveryAlns_tests)