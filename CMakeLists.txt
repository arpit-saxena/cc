cmake_minimum_required(VERSION 3.13.4)
project(cc)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(MyParser c.y c.tab.cpp)
FLEX_TARGET(MyScanner c.l c.lex.cpp COMPILE_FLAGS "-l")
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

SET(CMAKE_CXX_FLAGS  "-std=c++17 -g ${CMAKE_CXX_FLAGS}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(POPT_DIR "${CMAKE_SOURCE_DIR}/cmake_modules")

FIND_LIBRARY(POPT_LIBRARY NAMES popt)

FILE(GLOB SOURCES *.cpp)

# Now build our tools
add_executable(cc ${SOURCES} ${BISON_MyParser_OUTPUTS} ${FLEX_MyScanner_OUTPUTS})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs core)

# Link against LLVM libraries
target_link_libraries(cc ${llvm_libs} ${POPT_LIBRARY})

add_custom_target(build_tests)
add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND})
add_dependencies(run_tests build_tests)

enable_testing()

add_subdirectory(tests)