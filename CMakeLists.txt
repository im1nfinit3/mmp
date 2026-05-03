cmake_minimum_required(VERSION 3.10)

project(mmp LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_policy(SET CMP0076 NEW)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-pbutils-1.0)

find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

set(GRESOURCE_XML ${CMAKE_CURRENT_SOURCE_DIR}/src/mmp.gresource.xml)
set(GRESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/mmp-resources.c)

add_custom_command(
    OUTPUT ${GRESOURCE_C}
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
        --target=${GRESOURCE_C}
        --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/src
        --generate-source
        ${GRESOURCE_XML}
    DEPENDS
        ${GRESOURCE_XML}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/window.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/style.css
    VERBATIM
)

set(SOURCES
    src/main.c
    src/ui.c
    src/ui_callbacks.c
    src/playback.c
    src/database.c
    src/sqlite/sqlite3-all.c
    ${GRESOURCE_C}
)

add_executable(mmp ${SOURCES})
target_link_libraries(mmp PRIVATE
    ${GTK4_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
)
target_link_directories(mmp PRIVATE
    ${GTK4_LIBRARY_DIRS}
    ${GSTREAMER_LIBRARY_DIRS}
)

target_include_directories(mmp PRIVATE
    include/
    src/sqlite/
    ${GTK4_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
)
