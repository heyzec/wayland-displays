find_package(Catch2 3 REQUIRED)

add_executable(unit-tests
    server/handlers/WayDisplaysHandler/test_find_displays.cpp
)

# Test framework dependencies.
target_link_libraries(unit-tests PRIVATE Catch2::Catch2WithMain)

# Functionality to test
target_link_libraries(unit-tests PRIVATE handlers)

add_test(NAME unit-tests COMMAND unit-tests)
