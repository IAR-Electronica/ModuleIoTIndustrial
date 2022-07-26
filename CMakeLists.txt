# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
#list(APPEND EXTRA_COMPONENT_DIRS components)
include($ENV{MDF_PATH}/project.cmake)

#if(NOT DEFINED $ENV{MDF_PATH})
#    set(ENV{MDF_PATH} ${CMAKE_CURRENT_LIST_DIR}/../../..)
#endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #; third component used 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(new_prh)
