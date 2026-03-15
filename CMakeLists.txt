cmake_minimum_required(VERSION 3.10)

project(physics_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -g)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(main ${SOURCES})

set(_default_sdl_root "")
set(_sdl_arch "")

if(CMAKE_OSX_ARCHITECTURES)
    list(GET CMAKE_OSX_ARCHITECTURES 0 _sdl_arch)
elseif(CMAKE_SYSTEM_PROCESSOR)
    set(_sdl_arch "${CMAKE_SYSTEM_PROCESSOR}")
endif()

if(APPLE)
    if(_sdl_arch MATCHES "^(arm64|aarch64)$" AND EXISTS "${CMAKE_SOURCE_DIR}/SDL/sdl-mac-arm64")
        set(_default_sdl_root "${CMAKE_SOURCE_DIR}/SDL/sdl-mac-arm64")
    endif()
elseif(WIN32)
    if(EXISTS "${CMAKE_SOURCE_DIR}/SDL/sdl-win64")
        set(_default_wxwidgets_root "${CMAKE_SOURCE_DIR}/SDL/sdl-win64")
    endif()
endif()

set(SDL_ROOT "${_default_sdl_root}" CACHE PATH "Path to SDL install prefix")
if(NOT SDL_ROOT AND _default_sdl_root)
    set(SDL_ROOT "${_default_sdl_root}" CACHE PATH "Path to SDL install prefix" FORCE)
endif()

if(NOT SDL_ROOT)
    message(FATAL_ERROR "SDL_ROOT not set. Pass -DSDL_ROOT=/path/to/sdl-install to cmake.")
endif()

# Find SDL3 from the install prefix
find_library(SDL3_LIB
    NAMES SDL3
    PATHS "${SDL_ROOT}/lib"
    NO_DEFAULT_PATH
    REQUIRED
)

target_include_directories(main PRIVATE
    src/inc
    inc
    "${SDL_ROOT}/include"
)

target_link_libraries(main PRIVATE "${SDL3_LIB}")

# On macOS embed rpath so the binary finds the dylib at runtime
if(APPLE)
    set_target_properties(main PROPERTIES
        BUILD_RPATH "${SDL_ROOT}/lib"
        INSTALL_RPATH "${SDL_ROOT}/lib"
    )
endif()

# On Windows, copy SDL3.dll next to the executable after build
if(WIN32)
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL_ROOT}/bin/SDL3.dll"
            $<TARGET_FILE_DIR:main>
    )
endif()
