##########################################################################
cmake_minimum_required(VERSION 3.0.8)
##########################################################################
project(miyo-tool-img2arr)
##########################################################################
set(MIYO_TOOL_IMG2ARR_TARGET miyo-img2arr)
##########################################################################
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS program_options system REQUIRED)
##########################################################################
include_directories(
  ${OpenCV_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)
##########################################################################
add_executable(${MIYO_TOOL_IMG2ARR_TARGET}
  main.cpp
)
##########################################################################
target_compile_options(${MIYO_TOOL_IMG2ARR_TARGET} PUBLIC -Wall -Wextra -Wpedantic -Werror)
##########################################################################
target_link_libraries(${MIYO_TOOL_IMG2ARR_TARGET} ${OpenCV_LIBS} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SYSTEM_LIBRARY})
##########################################################################
