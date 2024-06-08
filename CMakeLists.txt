cmake_minimum_required(VERSION 3.26)

project(wayland-displays VERSION 1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_executable(wayland-displays src/main.cpp)

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
# Install
########################################

install(TARGETS wayland-displays DESTINATION bin)
