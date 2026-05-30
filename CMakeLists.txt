cmake_minimum_required(VERSION 3.14)
project(BotRunner VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find SFML 3
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

set(SOURCES
    src/main.cpp
    src/core/Camera.cpp
    src/math/Vector3.cpp
    src/math/Vector4.cpp
    src/math/Matrix4x4.cpp
    src/renderer/Renderer.cpp
    src/world/TextureAtlas.cpp
    src/world/Cube.cpp
)

add_executable(3D-game ${SOURCES})

target_include_directories(3D-game PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(3D-game PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)