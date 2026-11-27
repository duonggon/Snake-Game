cmake_minimum_required(VERSION 3.28)
project(SnakeBreaker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries")

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SFML)

add_library(SnakeCore Core/Map.cpp Core/Wall.cpp)
target_include_directories(SnakeCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SnakeCore PUBLIC SFML::Graphics)

add_executable(SnakeBreaker main.cpp Core/Game.cpp)
target_link_libraries(SnakeBreaker PRIVATE SnakeCore)
# Tim asset khi chay tu thu muc du an hoac thu muc build.
target_compile_definitions(SnakeBreaker PRIVATE SNAKEBREAKER_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
add_custom_command(TARGET SnakeBreaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:SnakeBreaker>/assets"
)

include(CTest)
if(BUILD_TESTING)
    add_executable(CoreTests tests/CoreTests.cpp)
    target_link_libraries(CoreTests PRIVATE SnakeCore)
    add_test(NAME CoreTests COMMAND CoreTests)
endif()
