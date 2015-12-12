CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT (turbo-linecount)

IF(MSVC)
	SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited configs" FORCE)
	SET(CMAKE_CXX_FLAGS_RELEASE "/Ox /Ob2")
	SET(CMAKE_C_FLAGS_RELEASE "/Ox /Ob2")
ELSE()
	SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
	SET(CMAKE_C_FLAGS_RELEASE "-O3")
ENDIF()

ADD_LIBRARY (linecount SHARED src/turbo_linecount.cpp src/turbo_linecount.h)
ADD_LIBRARY (linecount_static STATIC src/turbo_linecount.cpp src/turbo_linecount.h) 
ADD_EXECUTABLE (lc src/main.cpp)
TARGET_LINK_LIBRARIES (lc linecount_static)

INSTALL(TARGETS lc
        DESTINATION "bin"
        )
INSTALL(TARGETS linecount linecount_static
        DESTINATION "lib"
        )
INSTALL(FILES linecount.h
        DESTINATION "include"
        )
