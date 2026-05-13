add_library(sqlite STATIC sqlite3-all.c)
target_include_directories(sqlite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
