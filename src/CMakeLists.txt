set(SOURCES
    gui/canvas.cpp
    outputs/outputs.cpp
    outputs/head.cpp
    outputs/config.cpp
    outputs/mode.cpp
    utils/fixed24_8.cpp
    display.cpp
    main.cpp
)

# Create a library from the source files
add_library(handlers
    ${CMAKE_SOURCE_DIR}/src/server/handlers/WayDisplaysHandler/utils.cpp
)

list(TRANSFORM SOURCES PREPEND "src/")

