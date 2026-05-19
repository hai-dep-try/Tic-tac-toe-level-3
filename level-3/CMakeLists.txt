cmake_minimum_required(VERSION 3.16)

# 1. Project name & C++ standard
project(TicTacToe-LV3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Recursively gather sources from src/ — TRỪ recursion_combinator.cpp
#    (file demo Y combinator có main() riêng, không link vào game).
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*recursion_combinator\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/sdl/.*")

# 3. SDL — chỉ kích hoạt nếu sinh viên muốn giữ GUI từ level 2.
#    Mặc định TẮT để build sạch trên máy không có SDL.
option(USE_SDL "Build with SDL2 GUI" OFF)

# Đường dẫn tới SDL2 local (nếu có)
set(SDL2_LOCAL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../libs")

if(USE_SDL)
    # Ưu tiên tìm SDL2 từ local libs trước
    if(EXISTS "${SDL2_LOCAL_ROOT}/SDL2-2.30.12/cmake/sdl2-config.cmake")
        set(SDL2_DIR "${SDL2_LOCAL_ROOT}/SDL2-2.30.12/cmake")
        set(SDL2_ttf_DIR "${SDL2_LOCAL_ROOT}/SDL2_ttf-2.22.0/cmake")
    endif()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    # Them SDL sources
    file(GLOB_RECURSE SDL_SOURCES CONFIGURE_DEPENDS "src/sdl/*.cpp")
    list(APPEND SOURCES ${SDL_SOURCES})
endif()

# 4. Executable target
add_executable(game ${SOURCES})

# 5. Include paths & compile definitions
target_include_directories(game PRIVATE
    src
    $<$<BOOL:${USE_SDL}>:${SDL2_LOCAL_ROOT}/SDL2-2.30.12/x86_64-w64-mingw32/include/SDL2>
    $<$<BOOL:${USE_SDL}>:${SDL2_LOCAL_ROOT}/SDL2_ttf-2.22.0/x86_64-w64-mingw32/include/SDL2>
)

if(USE_SDL)
    target_compile_definitions(game PRIVATE USE_SDL)
endif()

# 6. Link libraries
if(USE_SDL)
    if(WIN32)
        target_link_libraries(game PRIVATE
            mingw32
            "${SDL2_LOCAL_ROOT}/SDL2-2.30.12/x86_64-w64-mingw32/lib/libSDL2main.a"
            "${SDL2_LOCAL_ROOT}/SDL2-2.30.12/x86_64-w64-mingw32/lib/libSDL2.dll.a"
            "${SDL2_LOCAL_ROOT}/SDL2_ttf-2.22.0/x86_64-w64-mingw32/lib/libSDL2_ttf.dll.a"
        )
    else()
        target_link_libraries(game PRIVATE
            SDL2::SDL2
            SDL2_ttf::SDL2_ttf
        )
    endif()
endif()

# 7. Một số cờ cần cho mức 3 nhánh B (parallel STL):
#    - GCC/Clang: cần TBB ở runtime cho std::execution::par.
#    - MSVC:     cần /EHsc.
if(MSVC)
    target_compile_options(game PRIVATE /W4 /O2 /EHsc)
else()
    target_compile_options(game PRIVATE -Wall -Wextra -O2)
    # Thread support (cho std::async / std::thread).
    find_package(Threads REQUIRED)
    target_link_libraries(game PRIVATE Threads::Threads)

    # Nếu sinh viên dùng std::execution::par, có thể cần TBB:
    #   sudo apt install libtbb-dev   (Ubuntu/Debian)
    # Bỏ comment 2 dòng sau nếu cần.
    # find_package(TBB REQUIRED)
    # target_link_libraries(game PRIVATE TBB::tbb)
endif()

# 8. Output binary tại thư mục source (giống level 2)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# 9. Copy assets/ vào cạnh binary nếu thư mục tồn tại (chỉ cần cho SDL)
if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:game>/assets
    )
endif()

# 10. Copy SDL2 DLLs vào thư mục binary (Windows)
if(USE_SDL AND WIN32)
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL2_LOCAL_ROOT}/SDL2-2.30.12/x86_64-w64-mingw32/bin/SDL2.dll"
            "${SDL2_LOCAL_ROOT}/SDL2_ttf-2.22.0/x86_64-w64-mingw32/bin/SDL2_ttf.dll"
            $<TARGET_FILE_DIR:game>
    )
endif()

# ──────────────────────────────────────────────────────────────────────
# (TUỲ CHỌN) Build target riêng cho demo Y combinator (mức 3 nhánh A)
#   cmake --build . --target y_demo
# ──────────────────────────────────────────────────────────────────────
add_executable(y_demo EXCLUDE_FROM_ALL src/core/recursion_combinator.cpp)
target_compile_definitions(y_demo PRIVATE RUN_Y_DEMO)
