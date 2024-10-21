cmake_minimum_required(VERSION 3.10)
project(xtrack CXX C)

# Set project directory
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Set LVGL directory
set(SIM_DIR_NAME lvgl CACHE STRING "Simulator directory name")
set(SIM_DIR ${PROJECT_DIR}/Simulator/LVGL.Simulator CACHE PATH "LVGL Simulator directory")


# Compiler flags
set(WARNING_FLAGS
    -Wall -Wextra -Wshadow -Wundef -Wmaybe-uninitialized
    -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith
    -fno-strict-aliasing -Wno-error=cpp -Wuninitialized
    -Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral
    -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type
    -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic
    -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion
    -Wclobbered -Wdeprecated -Wempty-body -Wshift-negative-value
    -Wstack-usage=4096 -Wtype-limits -Wsizeof-pointer-memaccess -Wpointer-arith
)

add_compile_options(
    -Wall -Wextra -Wshadow -Wundef -Wmaybe-uninitialized
    -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing
    -Wno-error=cpp -Wuninitialized -Wno-unused-parameter -Wno-missing-field-initializers
    -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default
    -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers
    -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes
    -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wshift-negative-value
    -Wstack-usage=4096 -Wtype-limits -Wsizeof-pointer-memaccess -Wpointer-arith
)

# Include directories
include_directories(
    ${PROJECT_DIR}/USER/App
    ${PROJECT_DIR}/USER/App/Utils/ArduinoJson/src
    ${SIM_DIR}
)

# Compiler definitions
add_definitions(
    -DLV_COLOR_DEPTH=32
    -DLV_COLOR_16_SWAP=0
    -DLV_FS_PC_PATH="../../../"
    -DCONFIG_MAP_EXT_NAME_DEFAULT="png"
)

# Set C and CXX flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g0")

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Check for lv_conf.h and lv_drv_conf.h
if(NOT EXISTS "${SIM_DIR}/lv_conf.h")
    message(FATAL_ERROR "lv_conf.h not found in the project directory. Please provide this file.")
endif()

if(NOT EXISTS "${SIM_DIR}/lv_drv_conf.h")
    message(FATAL_ERROR "lv_drv_conf.h not found in the project directory. Please provide this file.")
endif()

# Add LVGL library
add_subdirectory(${SIM_DIR}/lvgl ${CMAKE_BINARY_DIR}/lvgl)
# Ensure lv_conf.h is found
target_include_directories(lvgl PUBLIC ${SIM_DIR})

# Add lv_drivers library
add_subdirectory(${SIM_DIR}/lv_drivers ${CMAKE_BINARY_DIR}/lv_drivers)
target_include_directories(lv_drivers PUBLIC ${SIM_DIR})

# Collect source files
file(GLOB_RECURSE LVGL_FS_IF_SOURCES "${SIM_DIR}/lv_fs_if/*.c")
file(GLOB_RECURSE USER_APP_C_SOURCES "${PROJECT_DIR}/USER/App/*.c")
file(GLOB_RECURSE HAL_SOURCES "${SIM_DIR}/HAL/*.cpp")
file(GLOB_RECURSE USER_APP_CPP_SOURCES "${PROJECT_DIR}/USER/App/*.cpp")

# Filter out unwanted sources
list(FILTER USER_APP_CPP_SOURCES EXCLUDE REGEX "${PROJECT_DIR}/USER/App/Utils/ArduinoJson/.*\\.cpp$")
list(FILTER USER_APP_CPP_SOURCES EXCLUDE REGEX "${PROJECT_DIR}/USER/App/Utils/lv_img_png/PNGdec/linux/.*\\.cpp$")

# Add executable
add_executable(${PROJECT_NAME}
    main.cpp
    mouse_cursor_icon.c
    ${LVGL_SOURCES}
    ${LV_DRIVERS_SOURCES}
    ${LVGL_FS_IF_SOURCES}
    ${USER_APP_C_SOURCES}
    ${HAL_SOURCES}
    ${USER_APP_CPP_SOURCES}
)


#target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS} -O3 -g0)

# Add define to use custom lv_conf.h
target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    m
    stdc++
    SDL2::SDL2
)
