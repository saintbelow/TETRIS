cmake_minimum_required(VERSION 3.16)
project(neon_tetris C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ─── Executable ───
add_executable(neon_tetris src/main.c)
target_include_directories(neon_tetris PRIVATE include)

if(NOT MSVC)
    target_compile_options(neon_tetris PRIVATE -Wall -Wextra -O2)
endif()

# ─── Find dependencies ───
find_package(OpenGL REQUIRED)

if(WIN32)
    # MSYS2 / MinGW — use pkg-config for GLFW, GLEW, OpenAL
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)
    pkg_check_modules(GLEW REQUIRED glew)
    pkg_check_modules(OPENAL REQUIRED openal)

    target_include_directories(neon_tetris PRIVATE
        ${GLFW3_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${OPENAL_INCLUDE_DIRS})
    target_link_directories(neon_tetris PRIVATE
        ${GLFW3_LIBRARY_DIRS} ${GLEW_LIBRARY_DIRS} ${OPENAL_LIBRARY_DIRS})
    target_link_libraries(neon_tetris PRIVATE
        ${GLFW3_LIBRARIES} ${GLEW_LIBRARIES} OpenGL::GL ${OPENAL_LIBRARIES}
        gdi32 user32 shell32 imm32 winmm)

elseif(APPLE)
    # macOS — Homebrew + system frameworks
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)
    pkg_check_modules(GLEW REQUIRED glew)
    find_library(OPENAL_LIBRARY OpenAL REQUIRED)

    target_include_directories(neon_tetris PRIVATE
        ${GLFW3_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
    target_link_directories(neon_tetris PRIVATE
        ${GLFW3_LIBRARY_DIRS} ${GLEW_LIBRARY_DIRS})
    target_link_libraries(neon_tetris PRIVATE
        ${GLFW3_LIBRARIES} ${GLEW_LIBRARIES} OpenGL::GL ${OPENAL_LIBRARY})

else()
    # Linux
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)
    pkg_check_modules(GLEW REQUIRED glew)
    pkg_check_modules(OPENAL REQUIRED openal)

    target_include_directories(neon_tetris PRIVATE
        ${GLFW3_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${OPENAL_INCLUDE_DIRS})
    target_link_directories(neon_tetris PRIVATE
        ${GLFW3_LIBRARY_DIRS} ${GLEW_LIBRARY_DIRS} ${OPENAL_LIBRARY_DIRS})
    target_link_libraries(neon_tetris PRIVATE
        ${GLFW3_LIBRARIES} ${GLEW_LIBRARIES} OpenGL::GL ${OPENAL_LIBRARIES} m)
endif()

# ─── Install target ───
install(TARGETS neon_tetris RUNTIME DESTINATION bin)
