add_library(mmp_ui STATIC ui.c ui_callbacks.c)
target_include_directories(mmp_ui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GTK4_INCLUDE_DIRS}
)
target_link_libraries(mmp_ui PUBLIC
    mmp_core
    ${GTK4_LIBRARIES}
)
