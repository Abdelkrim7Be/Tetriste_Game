cmake_minimum_required(VERSION 3.10)
project(Tetriste VERSION 1.4)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window audio system REQUIRED)

# Source files
set(SOURCES
    main.cpp
    Game.cpp
    Piece.cpp
    AssetManager.cpp
    Renderer.cpp
)

# Executable
add_executable(Tetriste ${SOURCES})

# Include directories
target_include_directories(Tetriste PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
target_link_libraries(Tetriste PRIVATE sfml-graphics sfml-window sfml-audio sfml-system)

# Copy assets to build directory (optional but helpful for local running)
add_custom_command(TARGET Tetriste POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Tetriste>/assets
)
