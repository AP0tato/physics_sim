cmake_minimum_required(VERSION 3.10)

project(physics_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -g)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

if(APPLE)
    list(APPEND SOURCES src/gpu-compute/metal/wrapper.mm)
endif()

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
        set(_default_sdl_root "${CMAKE_SOURCE_DIR}/SDL/sdl-win64")
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

find_library(SDL3_TTF_LIB
    NAMES SDL3_ttf SDL3_ttf-static
    PATHS "${SDL_ROOT}/lib"
    NO_DEFAULT_PATH
)

find_path(SDL3_TTF_INCLUDE_DIR
    NAMES SDL3_ttf/SDL_ttf.h SDL_ttf.h
    PATHS "${SDL_ROOT}/include"
    NO_DEFAULT_PATH
)

target_include_directories(main PRIVATE
    src/inc
    src
    "${SDL_ROOT}/include"
)

target_link_libraries(main PRIVATE "${SDL3_LIB}")

if(SDL3_TTF_LIB AND SDL3_TTF_INCLUDE_DIR)
    target_link_libraries(main PRIVATE "${SDL3_TTF_LIB}")
    target_compile_definitions(main PRIVATE PHYSICS_HAS_SDL_TTF=1)
else()
    target_compile_definitions(main PRIVATE PHYSICS_HAS_SDL_TTF=0)
    message(WARNING "SDL3_ttf header/library pair not found under SDL_ROOT. Button text rendering is disabled until SDL_ttf is installed in the same prefix.")
endif()

if(APPLE)
    target_link_libraries(main PRIVATE
        "-framework Metal"
        "-framework Foundation"
    )

    find_program(XCRUN_EXECUTABLE xcrun REQUIRED)
    execute_process(
        COMMAND "${XCRUN_EXECUTABLE}" -sdk macosx metal -v
        RESULT_VARIABLE METAL_COMPILER_STATUS
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(METAL_COMPILER_STATUS EQUAL 0)
        set(METAL_SOURCE "${CMAKE_SOURCE_DIR}/src/gpu-compute/metal/functions.metal")
        set(METAL_AIR "${CMAKE_CURRENT_BINARY_DIR}/functions.air")
        set(METAL_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/functions.metallib")

        add_custom_command(
            OUTPUT "${METAL_LIBRARY}"
            COMMAND "${XCRUN_EXECUTABLE}" -sdk macosx metal -c "${METAL_SOURCE}" -o "${METAL_AIR}"
            COMMAND "${XCRUN_EXECUTABLE}" -sdk macosx metallib "${METAL_AIR}" -o "${METAL_LIBRARY}"
            DEPENDS "${METAL_SOURCE}"
            COMMENT "Compiling Metal shaders"
            VERBATIM
        )

        add_custom_target(metal_shaders ALL DEPENDS "${METAL_LIBRARY}")
        add_dependencies(main metal_shaders)

        add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${METAL_LIBRARY}"
                "$<TARGET_FILE_DIR:main>/functions.metallib"
        )
    else()
        message(WARNING "Metal command-line toolchain is unavailable. Install it with: xcodebuild -downloadComponent MetalToolchain")
    endif()
endif()

# On macOS embed rpath so the binary finds the dylib at runtime
if(APPLE)
    set_target_properties(main PROPERTIES
        BUILD_RPATH "${SDL_ROOT}/lib"
        INSTALL_RPATH "${SDL_ROOT}/lib"
    )
endif()

# On Windows, copy SDL3.dll next to the executable after build
if(WIN32)
    target_link_libraries(main PRIVATE dxgi)

    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL_ROOT}/bin/SDL3.dll"
            $<TARGET_FILE_DIR:main>
    )

    if(SDL3_TTF_LIB AND EXISTS "${SDL_ROOT}/bin/SDL3_ttf.dll")
        add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL_ROOT}/bin/SDL3_ttf.dll"
                $<TARGET_FILE_DIR:main>
        )
    endif()
endif()
