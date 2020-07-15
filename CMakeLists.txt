cmake_minimum_required(VERSION 3.0.0)
project(example_module LANGUAGES C)

# Find kernel headers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(KernelHeaders REQUIRED)
include_directories(${KERNELHEADERS_INCLUDE_DIRS})

# Module file name
set(MODULE_FILE "${PROJECT_NAME}.ko")
set(KBUILD_FILE "obj-m := ${PROJECT_NAME}.o")

# Module build command
set(KBUILD_CMD $(MAKE) -C ${KERNELHEADERS_DIR} modules M=${CMAKE_CURRENT_BINARY_DIR} src=${CMAKE_CURRENT_SOURCE_DIR})

# Get a list of all source files
file(GLOB_RECURSE sources_absolute ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
foreach(file ${sources_absolute})
  # Extract file components
  file(RELATIVE_PATH file "${CMAKE_CURRENT_SOURCE_DIR}" "${file}")
  get_filename_component(file_dir "${file}" DIRECTORY)
  get_filename_component(file_name_we "${file}" NAME_WE)
  
  # Create the build directory structure
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${file_dir})

  # Append the file to Kbuild
  set(KBUILD_FILE "${KBUILD_FILE}\n${PROJECT_NAME}-y += ${file_dir}/${file_name_we}.o")

  # Append to relative sources list
  list(APPEND sources_relative "${file_dir}/${file_name_we}.c")
endforeach()

# Append build flags to Kbuild
set(KBUILD_FILE "${KBUILD_FILE}\nccflags-y := -DMODULE_NAME=\\\"${PROJECT_NAME}\\\"")

# Strip trailing whitespaces from ${sources_relative}
string(REGEX REPLACE "\n$" "" ${sources_relative} "${sources_relative}")

# Write the Kbuild file
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/Kbuild ${KBUILD_FILE})

# Create a custom command for running the module build
add_custom_command(OUTPUT ${MODULE_FILE}
  COMMAND ${KBUILD_CMD}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${sources_relative} VERBATIM)

add_custom_target(${PROJECT_NAME} ALL DEPENDS ${MODULE_FILE})
