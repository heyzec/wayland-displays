cmake_minimum_required(VERSION 3.26)

# The compiler and linker is usually determined by the file extension if not set otherwise
project(wayland-displays VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_executable(wayland-displays protocols/wlr-output-management-unstable-v1.c src/main.cpp)

include_directories(
  .
  "protocols/"
  "src/")

########################################
# Dependencies
########################################

# Both Cairo and GTK3 require their own FindXXX.cmake as they are not present in
# https://gitlab.kitware.com/cmake/cmake/tree/master/Modules

find_package(Cairo REQUIRED)
include_directories(${CAIRO_INCLUDE_DIRS})
target_link_libraries(wayland-displays ${CAIRO_LIBRARIES})

find_package(GTK3 REQUIRED)
include_directories(${GTK3_INCLUDE_DIRS})
target_link_libraries(wayland-displays ${GTK3_LIBRARIES})

find_package(Wayland REQUIRED)
include_directories(${WAYLAND_INCLUDE_DIRS})
target_link_libraries(wayland-displays ${WAYLAND_LIBRARIES})

########################################
# Generated protocols
########################################

pkg_get_variable(WaylandScanner wayland-scanner wayland_scanner)

add_custom_target(generate-protocol-headers)

function(protocol protoName)
    set(path ${CMAKE_SOURCE_DIR}/protocols)

    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/protocols/${protoName}.h
        COMMAND wayland-scanner client-header ${path}/${protoName}.xml protocols/${protoName}.h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/protocols/${protoName}.c
        COMMAND wayland-scanner private-code ${path}/${protoName}.xml protocols/${protoName}.c
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    target_sources(generate-protocol-headers PRIVATE ${CMAKE_SOURCE_DIR}/protocols/${protoName}.h)
    target_sources(wayland-displays PRIVATE ${CMAKE_SOURCE_DIR}/protocols/${protoName}.h)
endfunction()

protocol("wlr-output-management-unstable-v1")

add_dependencies(wayland-displays generate-protocol-headers)

########################################
# Install
########################################

install(TARGETS wayland-displays DESTINATION bin)
