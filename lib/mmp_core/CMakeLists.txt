add_library(mmp_core STATIC playback.c library.c database.c song.c)
target_include_directories(mmp_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${GTK4_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
)
target_link_libraries(mmp_core PUBLIC
    sqlite
    ${GSTREAMER_LIBRARIES}
)
