cmake_minimum_required(VERSION 3.26)

# The compiler and linker is usually determined by the file extension if not set otherwise
project(wayland-displays VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Get ${SOURCES}
include(src/CMakeLists.txt)
# Get ${LIB_SOURCES}
include(lib/CMakeLists.txt)

add_executable(wayland-displays ${SOURCES} ${LIB_SOURCES})

target_link_libraries(wayland-displays PRIVATE handlers)

# Add the main source of headers
include_directories("src/")
include_directories("lib/")

########################################
# Tests
########################################

# Must be set before including CTest so that these arguments are applied when
# running `$ ninja test` or `$ make test`.
set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")

# Enable CTest support, see https://cmake.org/cmake/help/latest/module/CTest.html
include(CTest)

# Add testing code only if this variable is ON
if(BUILD_TESTING)
  add_subdirectory(tests/unit)
endif()

########################################
# Dependencies
########################################

# Both Cairo and GTK3 require their own FindXXX.cmake as they are not present in
# https://gitlab.kitware.com/cmake/cmake/tree/master/Modules

find_package(Cairo REQUIRED)
include_directories(${CAIRO_INCLUDE_DIRS})
target_link_libraries(wayland-displays PUBLIC ${CAIRO_LIBRARIES})

find_package(GTK3 REQUIRED)
include_directories(${GTK3_INCLUDE_DIRS})
target_link_libraries(wayland-displays PUBLIC ${GTK3_LIBRARIES})

find_package(Wayland REQUIRED)
include_directories(${WAYLAND_INCLUDE_DIRS})
target_link_libraries(wayland-displays PUBLIC ${WAYLAND_LIBRARIES})

pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
include_directories(${YAML_CPP_INCLUDE_DIRS})
target_link_libraries(wayland-displays PUBLIC ${YAML_CPP_LIBRARIES})

find_package(spdlog REQUIRED)
target_link_libraries(wayland-displays PRIVATE spdlog::spdlog)
add_compile_definitions(SPDLOG_FMT_EXTERNAL)

########################################
# Generated protocols
########################################

add_custom_target(generate-protocol)
add_dependencies(wayland-displays generate-protocol)

set(PROTO_SRC ${CMAKE_SOURCE_DIR}/protocols)
set(PROTO_OUT ${CMAKE_BINARY_DIR}/protocols)

# Find wayland-scanner binary
pkg_get_variable(WaylandScanner wayland-scanner wayland_scanner)

set(GENERATED_SOURCES "")
set(GENERATED_HEADERS "")

function(add_protocol protoName)
    file(MAKE_DIRECTORY ${PROTO_OUT})
    set(PROTOCOL_H ${PROTO_OUT}/${protoName}.h)
    set(PROTOCOL_C ${PROTO_OUT}/${protoName}.c)

    # Generate header and source files
    add_custom_command(
        OUTPUT ${PROTOCOL_H}
        COMMAND wayland-scanner client-header ${PROTO_SRC}/${protoName}.xml ${PROTOCOL_H}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_custom_command(
        OUTPUT ${PROTOCOL_C}
        COMMAND wayland-scanner private-code ${PROTO_SRC}/${protoName}.xml ${PROTOCOL_C}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    list(APPEND GENERATED_HEADERS ${PROTOCOL_H})
    list(APPEND GENERATED_SOURCES ${PROTOCOL_C})

    set(GENERATED_HEADERS "${GENERATED_HEADERS}" PARENT_SCOPE)
    set(GENERATED_SOURCES "${GENERATED_SOURCES}" PARENT_SCOPE)
endfunction()

# Add all needed protocols here
# TODO: glob and foreach
add_protocol("wlr-output-management-unstable-v1")
add_protocol("wlr-screencopy-unstable-v1")
message("${GENERATED_HEADERS}")

target_sources(generate-protocol PRIVATE ${GENERATED_HEADERS})
add_library(generate-protocol-sources STATIC ${GENERATED_SOURCES})

# Include the generated protocol headers
include_directories(${PROTO_OUT})
# Link the generated protocol source files
target_link_libraries(wayland-displays PUBLIC generate-protocol-sources)


########################################
# Generate resources
########################################

add_custom_target(generate-resources)
add_dependencies(wayland-displays generate-resources)

# Add the resources
set(RESOURCE_XML ${CMAKE_SOURCE_DIR}/resources/resources.xml)
set(RESOURCE_OUT ${CMAKE_BINARY_DIR}/resources)
file(MAKE_DIRECTORY ${RESOURCE_OUT})
set(RESOURCE_C ${RESOURCE_OUT}/resources.c)

# Custom command to compile the resources
add_custom_command(
    OUTPUT ${RESOURCE_C}
    COMMAND glib-compile-resources --target=${RESOURCE_C} --generate-source ${RESOURCE_XML}
    DEPENDS ${RESOURCE_XML} ${CMAKE_SOURCE_DIR}/resources/layout.ui
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/resources
    COMMENT "Compiling resources"
)

add_library(generate-resources-sources STATIC ${RESOURCE_C})

include_directories(wayland-displays PRIVATE ${RESOURCE_OUT})

target_link_libraries(wayland-displays PUBLIC generate-resources-sources)


########################################
# Install
########################################

install(TARGETS wayland-displays DESTINATION bin)

# Hack: Add generated sources to include/ directory for LSP
install(DIRECTORY ${PROTO_OUT}/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")
    install(FILES ${RESOURCE_OUT}/resources.c
        DESTINATION include)
