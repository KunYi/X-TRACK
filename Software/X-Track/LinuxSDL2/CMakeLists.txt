cmake_minimum_required(VERSION 3.13)
project(xtrack CXX C)

# Set default compilers if not specified
if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER gcc)
endif()
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER g++)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")

# Define project structure
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(LVGL_DIR_NAME "lvgl" CACHE STRING "LVGL directory name")
set(LVGL_DIR "${PROJECT_DIR}/Simulator/LVGL.Simulator" CACHE PATH "LVGL directory")

find_package(SDL2 REQUIRED)

# Include LVGL and drivers CMake configurations
add_subdirectory(${LVGL_DIR}/lvgl ${CMAKE_BINARY_DIR}/lvgl_build)
add_subdirectory(${LVGL_DIR}/lv_drivers ${CMAKE_BINARY_DIR}/lv_drivers_build)

# Common definitions
set(COMMON_DEFINITIONS
    -DLV_HOR_RES=480
    -DLV_VER_RES=320
    -DUSE_SDL=1
    -DUSE_WIN32DRV=0
    -DUSE_EVDEV=0
    -DLV_COLOR_DEPTH=32
    -DLV_COLOR_16_SWAP=0
    -DLV_FS_PC_PATH="../../../"
)

# Target-specific configurations for LVGL and lv_drivers
target_compile_definitions(lvgl PRIVATE ${COMMON_DEFINITIONS})
target_compile_definitions(lv_drivers PRIVATE ${COMMON_DEFINITIONS})
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS} ${LVGL_DIR})
target_include_directories(lv_drivers PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS} ${LVGL_DIR})
target_link_libraries(lv_drivers PUBLIC ${SDL2_LIBRARIES})

# Compiler warnings
set(WARNING_FLAGS
    -Wall -Wextra -Wshadow -Wundef -Wmaybe-uninitialized -Wno-unused-function
    -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing
    -Wno-error=cpp -Wuninitialized -Wno-unused-parameter
    -Wno-missing-field-initializers -Wno-format-nonliteral
    -Wno-cast-qual -Wunreachable-code -Wno-switch-default
    -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers
    -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes
    -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body
    -Wshift-negative-value -Wstack-usage=4096 -Wtype-limits
    -Wsizeof-pointer-memaccess -Wpointer-arith
)

# Source files
file(GLOB_RECURSE LV_FS_IF_SOURCES "${LVGL_DIR}/lv_fs_if/*.c")
file(GLOB_RECURSE USER_APP_C_SOURCES "${PROJECT_DIR}/USER/App/*.c")
file(GLOB_RECURSE HAL_CPP_SOURCES "${LVGL_DIR}/HAL/*.cpp")
file(GLOB_RECURSE USER_APP_CPP_SOURCES "${PROJECT_DIR}/USER/App/*.cpp")

# Filter out specific directories
list(FILTER USER_APP_CPP_SOURCES EXCLUDE REGEX "(${PROJECT_DIR}/USER/App/Utils/ArduinoJson|${PROJECT_DIR}/USER/App/Utils/lv_img_png)")

# Add mouse cursor icon
set(ADDITIONAL_SOURCES mouse_cursor_icon.c)

# Create executable
add_executable(${PROJECT_NAME}
    main.cpp
    ${LV_FS_IF_SOURCES}
    ${USER_APP_C_SOURCES}
    ${HAL_CPP_SOURCES}
    ${USER_APP_CPP_SOURCES}
    ${ADDITIONAL_SOURCES}
)

# Set compiler options and flags
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${WARNING_FLAGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${WARNING_FLAGS}>
    -O3
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    m
    stdc++
    SDL2::SDL2
    lvgl         # From LVGL CMakeLists.txt
    lv_drivers   # From drivers CMakeLists.txt
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${LVGL_DIR}
    ${PROJECT_DIR}/USER/App
    ${PROJECT_DIR}/USER/App/Utils/ArduinoJson/src
)

# Target-specific definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMMON_DEFINITIONS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
