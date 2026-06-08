cmake_minimum_required(VERSION 3.14)
project(BotRunner VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FORCE_FETCH_SFML "Force download and build SFML even if system version exists" OFF)

if(APPLE)
    if(EXISTS "/opt/homebrew")
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
    elseif(EXISTS "/usr/local")
        list(APPEND CMAKE_PREFIX_PATH "/usr/local")
    endif()
endif()

if(NOT FORCE_FETCH_SFML)
    find_package(SFML 3 COMPONENTS Graphics Window System Network)
endif()

if(SFML_FOUND AND NOT FORCE_FETCH_SFML)
    message(STATUS "Found system SFML 3 at: ${SFML_DIR}")
    set(USE_SYSTEM_SFML TRUE)
else()
    message(STATUS "System SFML 3 not found. Downloading and building...")
    set(USE_SYSTEM_SFML FALSE)
    
    include(FetchContent)
    
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
    
    FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        GIT_PROGRESS ON
    )
    
    FetchContent_MakeAvailable(sfml)
endif()

set(SOURCES
    src/main.cpp
    src/core/Camera.cpp
    src/core/Game.cpp
    src/math/Vector3.cpp
    src/math/Vector4.cpp
    src/math/Matrix4x4.cpp
    src/renderer/Renderer.cpp
    src/world/TextureManager.cpp
    src/world/BlockRegistry.cpp
    src/world/BiomeManager.cpp
    src/world/Noise.cpp
    src/world/Chunk.cpp
    src/world/World.cpp
    src/world/Item.cpp
    src/world/Inventory.cpp
    src/world/InventoryUI.cpp
    src/world/PlayerInteraction.cpp
    src/world/PlayerView.cpp
    src/world/CraftingSystem.cpp
    src/menu/MenuWidget.cpp
    src/menu/MenuManager.cpp
    src/world/WorldSave.cpp
    src/network/Packet.cpp
    src/network/Server.cpp
    src/network/Client.cpp
)

add_executable(3D-game ${SOURCES})

target_include_directories(3D-game PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

if(USE_SYSTEM_SFML)
    target_link_libraries(3D-game PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Network
    )
else()
    target_link_libraries(3D-game PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-network
    )
endif()

if(WIN32)
    target_link_libraries(3D-game PRIVATE ws2_32)
endif()

add_custom_command(TARGET 3D-game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:3D-game>/assets"
    COMMENT "Copying assets to build directory..."
)

if(APPLE)
    set_target_properties(3D-game PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "VoxelEngine"
        MACOSX_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"
    )

    target_link_libraries(3D-game PRIVATE
        "-framework CoreFoundation"
    )
    
    add_custom_command(TARGET 3D-game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:3D-game>/Contents/Resources/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_BUNDLE_DIR:3D-game>/Contents/Resources/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/AppIcon.icns"
            "$<TARGET_BUNDLE_DIR:3D-game>/Contents/Resources/AppIcon.icns"
        COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:3D-game>"
        COMMENT "Copying assets to macOS app bundle and signing..."
    )
endif()

message(STATUS "")
message(STATUS "=== BotRunner Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if(USE_SYSTEM_SFML)
    message(STATUS "SFML: System installed version")
else()
    message(STATUS "SFML: Built from source (3.0.2)")
endif()
message(STATUS "================================")
message(STATUS "")