cmake_minimum_required(VERSION 3.16)
project(pocket-co-render C CXX)
set(CMAKE_C_STANDARD 11)
get_filename_component(APP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/lv-conf.h" CACHE PATH "" FORCE)
add_subdirectory("${APP_ROOT}/managed_components/lvgl__lvgl" lvgl)
add_executable(pocket-co-render render.c
    "${APP_ROOT}/main/pocket-co-ui.c"
    "${APP_ROOT}/main/pocket-co-content.c"
    "${APP_ROOT}/main/pocket-co-state.c"
    "${APP_ROOT}/assets/fonts/pocket-co-font-16.c"
    "${APP_ROOT}/assets/fonts/pocket-co-font-22.c"
    "${APP_ROOT}/assets/images/pocket-co-images.c"
    "${APP_ROOT}/components/bsp/src/bsp_display_rounding.c")
target_include_directories(pocket-co-render PRIVATE "${APP_ROOT}/main" "${APP_ROOT}/assets/images" "${APP_ROOT}/components/bsp/src")
target_link_libraries(pocket-co-render PRIVATE lvgl)
