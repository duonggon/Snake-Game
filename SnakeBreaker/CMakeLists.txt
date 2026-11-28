cmake_minimum_required(VERSION 3.16)
project(SnakeBreaker VERSION 1.0 LANGUAGES CXX)

# Sử dụng chuẩn C++17 theo đúng yêu cầu kỹ thuật của đồ án
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Danh sách các file mã nguồn của Thành viên 2 (Snake & Apple System)
set(PLAYER_SOURCES
    Player/Snake.cpp
    Player/Apple.cpp
    Player/NormalApple.cpp
    Player/HealthApple.cpp
    Player/EnergyApple.cpp
    Player/AppleSpawner.cpp
)

add_executable(SnakeBreaker
    main.cpp
    ${PLAYER_SOURCES}
)

target_include_directories(SnakeBreaker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Player
)

# Tự động tìm SFML 2.5+ (nếu máy đã cài đặt SFML thì tự động liên kết thư viện đồ họa)
if(MINGW)
    target_link_options(SnakeBreaker PRIVATE -static-libgcc -static-libstdc++ -static)
endif()

find_package(SFML 2.5 COMPONENTS graphics window system audio QUIET)
if(SFML_FOUND)
    message(STATUS "Found SFML: Building SnakeBreaker with SFML Graphics Window support.")
    target_link_libraries(SnakeBreaker PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)
else()
    message(STATUS "SFML not installed yet: Building SnakeBreaker in Standalone Unit-Test & Console Mode.")
endif()
