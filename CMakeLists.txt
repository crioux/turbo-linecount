cmake_minimum_required(VERSION 3.3.0)
project (linecount)

add_library (linecount SHARED linecount.cpp linecount.h)
add_library (linecount_static STATIC linecount.cpp linecount.h)
add_executable (lc main.cpp)
target_link_libraries (lc linecount_static)
