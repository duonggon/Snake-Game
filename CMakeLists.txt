cmake_minimum_required(VERSION 3.20)
project(SnakeCombatModule LANGUAGES CXX)

# SFML 3.1.0 bắt buộc sử dụng chuẩn C++17 trở lên
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Tự động trỏ tới SFML 3.1.0 cục bộ trong .toolchains nếu người dùng chưa truyền SFML_DIR riêng
set(LOCAL_SFML_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.toolchains/SFML-3.1.0/lib/cmake/SFML")
if(NOT DEFINED SFML_DIR AND EXISTS "${LOCAL_SFML_CMAKE_DIR}/SFMLConfig.cmake")
    set(SFML_DIR "${LOCAL_SFML_CMAKE_DIR}")
endif()

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# =============================================================================
# 1. THƯ VIỆN MODULE CỦA HUỲNH NHỰT CƯỜNG (Combat + Effects)
#    Khi ghép vào dự án chung của nhóm, chỉ cần giữ nguyên thư viện tĩnh này!
# =============================================================================
add_library(CombatAndEffects STATIC
    Effects/ScreenShake.cpp
    Effects/EffectManager.cpp
    Combat/CombatSystem.cpp
)

target_include_directories(CombatAndEffects PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(CombatAndEffects PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
)

if(MINGW)
    target_compile_options(CombatAndEffects PRIVATE -Wall -Wextra -Wpedantic)
endif()

# =============================================================================
# 2. BẢN CHƠI THỬ TRỰC QUAN ĐỘC LẬP (CombatSandbox.exe)
#    Cho phép chơi thử trọn vẹn cả Màn 1 (Sinh tồn) và Màn 2 (Đánh Boss 2x2)
# =============================================================================
add_executable(CombatSandbox
    Sandbox/CombatSandboxMain.cpp
)

target_link_libraries(CombatSandbox PRIVATE CombatAndEffects)

# Tự động chép các tệp .dll của SFML 3.1.0 & MinGW sang thư mục build/
# để bạn có thể bấm đúp chạy CombatSandbox.exe trực tiếp hoặc nhấn F5 trên VS Code
set(LOCAL_SFML_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.toolchains/SFML-3.1.0/bin")
set(LOCAL_MINGW_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.toolchains/mingw64/bin")

if(WIN32 AND EXISTS "${LOCAL_SFML_BIN_DIR}")
    file(GLOB SFML_RUNTIME_DLLS "${LOCAL_SFML_BIN_DIR}/*.dll")
    foreach(DLL_FILE ${SFML_RUNTIME_DLLS})
        add_custom_command(TARGET CombatSandbox POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL_FILE}" "$<TARGET_FILE_DIR:CombatSandbox>"
        )
    endforeach()
endif()

if(WIN32 AND EXISTS "${LOCAL_MINGW_BIN_DIR}")
    foreach(RUNTIME_DLL "libgcc_s_seh-1.dll" "libstdc++-6.dll" "libwinpthread-1.dll")
        if(EXISTS "${LOCAL_MINGW_BIN_DIR}/${RUNTIME_DLL}")
            add_custom_command(TARGET CombatSandbox POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${LOCAL_MINGW_BIN_DIR}/${RUNTIME_DLL}"
                    "$<TARGET_FILE_DIR:CombatSandbox>"
            )
        endif()
    endforeach()
endif()

# =============================================================================
# 3. BỘ KIỂM THỬ LOGIC TỰ ĐỘNG (CombatLogicTests.exe)
# =============================================================================
enable_testing()

add_executable(CombatLogicTests
    Tests/CombatLogicTests.cpp
)

target_link_libraries(CombatLogicTests PRIVATE CombatAndEffects)

add_test(NAME CombatLogicSuite COMMAND CombatLogicTests)
if(WIN32)
    set_tests_properties(CombatLogicSuite PROPERTIES
        ENVIRONMENT "PATH=${LOCAL_SFML_BIN_DIR};${LOCAL_MINGW_BIN_DIR};$ENV{PATH}"
    )
endif()
