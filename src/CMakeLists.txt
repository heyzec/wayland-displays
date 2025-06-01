set(SOURCES
    gui/canvas.cpp
    gui/details.cpp
    gui/screencopy.cpp
    gui/glarea.cpp
    outputs/outputs.cpp
    outputs/head.cpp
    outputs/config.cpp
    outputs/mode.cpp
    common/ipc.cpp
    common/logger.cpp
    common/paths.cpp
    common/shapes.cpp
    common/socket.cpp
    main.cpp
)

# Create a library from the source files
add_library(handlers
    ${CMAKE_SOURCE_DIR}/src/server/handlers/WayDisplaysHandler/utils.cpp
)

list(TRANSFORM SOURCES PREPEND "src/")

