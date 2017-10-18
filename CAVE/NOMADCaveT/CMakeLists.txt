cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Project Name
PROJECT(NOMADCaveT)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")        
else()
	MESSAGE(FATAL_ERROR "C++ 11 is required")
endif()

set(project_LIBRARIES)
set(project_INCLUDES)
set(project_DEFINITIONS -DCAVE -DINDICESGL32)

#########################################################
# ADD PTHREAD
#########################################################
list(APPEND project_LIBRARIES pthread)
find_package(synch REQUIRED)
#list(APPEND project_INCLUDES ${SYNCH_INCLUDE_DIRS})
list(APPEND project_LIBRARIES synch)

find_package(screen REQUIRED )

#plib
#find_library (PLIB_UL plibul HINTS $ENV{PLIBDIR} PATH_SUFFIXES lib REQUIRED)
#find_library (PLIB_FNT plibfnt HINTS $ENV{PLIBDIR} PATH_SUFFIXES lib REQUIRED)
#set (PLIB_LIBRARIES ${PLIB_FNT} ${PLIB_UL})
#message ("plib_libraries = ${PLIB_LIBRARIES} ")
#find_path (PLIB_INCLUDE_DIR plib/fnt.h HINTS $ENV{PLIBDIR} PATH_SUFFIXES include REQUIRED)
#message ("plib_include = ${PLIB_INCLUDE_DIR} ")

#freetype2
INCLUDE (FindPkgConfig)
pkg_search_module(FREETYPE REQUIRED freetype2)
message ("freetype LDFLAGS ${FREETYPE_LDFLAGS}")
message ("freetype CFLAGS ${FREETYPE_CFLAGS}")
#target_link_libraries(demoCO2CaO PUBLIC screen)
#target_link_libraries(demoCO2CaO_server PUBLIC screen)
#list(APPEND project_INCLUDES ${SCREEN_INCLUDE_DIRS})
list(APPEND project_LIBRARIES screen)



#########################################################
# Include Files
#########################################################
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)
include_directories(${project_INCLUDES})
include_directories(${FREETYPE_INCLUDE_DIRS})
set(HEADERS

)
set(SOURCES
       src/main.cpp src/textRendering.cpp
	src/NOMADVRLib/atoms.cpp	src/NOMADVRLib/CompileGLShader.cpp	
	src/NOMADVRLib/polyhedron.cpp   src/NOMADVRLib/UnitCellShaders.cpp
	src/NOMADVRLib/atomsGL.cpp	src/NOMADVRLib/ConfigFile.cpp	
	src/NOMADVRLib/TessShaders.cpp	src/NOMADVRLib/IsosurfacesGL.cpp
	src/NOMADVRLib/IsoShaders.cpp	src/NOMADVRLib/Grid.cpp
	src/NOMADVRLib/markerShaders.cpp
	src/happyhttp/happyhttp.cpp
	src/rply/rply.c
)



set(HEADERS_SERVER
)

set(SOURCES_SERVER
	src/main_server.cpp src/textRendering.cpp
)



########################################################
# Add definitions
#########################################################
add_definitions(${project_DEFINITIONS})

########################################################
# Linking & stuff
#########################################################


add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${project_LIBRARIES} ${FREETYPE_LDFLAGS})
add_executable(${PROJECT_NAME}_server ${SOURCES_SERVER} ${HEADERS_SERVER})
target_link_libraries(${PROJECT_NAME}_server ${project_LIBRARIES} ${FREETYPE_LIBRARIES})


