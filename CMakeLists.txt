cmake_minimum_required(VERSION 3.10)
project(Tetriste VERSION 1.4)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window audio system REQUIRED)

set(CORE_SOURCES
    src/core/Game.cpp
    src/core/Piece.cpp
)

set(PLATFORM_SOURCES
    src/platform/AssetManager.cpp
    src/platform/UserManager.cpp
)

set(UI_SOURCES
    src/ui/Renderer.cpp
)

set(APP_SOURCES
    src/main.cpp
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
    ${UI_SOURCES}
)

add_executable(Tetriste ${APP_SOURCES})

target_include_directories(Tetriste PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

target_link_libraries(Tetriste PRIVATE sfml-graphics sfml-window sfml-audio sfml-system)

add_custom_command(TARGET Tetriste POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Tetriste>/assets
)

# Optional standalone test binaries
add_executable(test_logic tests/test_logic.cpp ${CORE_SOURCES})
target_include_directories(test_logic PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)
add_executable(test_userManager tests/test_userManager.cpp src/platform/UserManager.cpp)
target_include_directories(test_userManager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
)
