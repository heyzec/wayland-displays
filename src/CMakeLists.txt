set(SOURCES
    gui/canvas.cpp
    outputs/outputs.cpp
    outputs/head.cpp
    outputs/config.cpp
    outputs/mode.cpp
    utils/fixed24_8.cpp
    server/handlers/WayDisplaysHandler/utils.cpp
    display.cpp
    main.cpp
)

list(TRANSFORM SOURCES PREPEND "src/")

