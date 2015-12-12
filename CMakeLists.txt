cmake_minimum_required(VERSION 3.3.0)
project (linecount)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_library (linecount SHARED linecount.cpp linecount.h)
add_library (linecount_static STATIC linecount.cpp linecount.h)
add_executable (lc main.cpp)
target_link_libraries (lc linecount_static)

install(TARGETS lc
        DESTINATION "bin"
        )
install(TARGETS linecount linecount_static
        DESTINATION "lib"
        )
install(FILES linecount.h
        DESTINATION "include"
        )
