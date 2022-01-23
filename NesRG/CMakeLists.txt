# CMakeList.txt : CMake project for NesRG, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set(CMAKE_CXX_FLAGS_DEBUG "/ZI")
	set(CMAKE_SHARED_LINKER_FLAGS "/SAFESEH:NO")
	set(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
	set(CMAKE_CXX_STANDARD 17)
endif(MSVC)

set (ROMS_DIR "roms")
set (TEST_DIR "tests")

set(TOP_DIR ${CMAKE_SOURCE_DIR})
set(SRC_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})
set(APP_DIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${SRC_DIR}/include)

if(WIN32)
	file(GLOB_RECURSE SRC_WIN
	${SRC_DIR}/src/*.cpp
	${SRC_DIR}/windows/*.cpp
	${SRC_DIR}/imgui/*.cpp
	${SRC_DIR}/main.cpp
	)

	include_directories(${SRC_DIR}/windows)	
	include_directories(${SRC_DIR}/imgui)

	add_executable (${PROJECT_NAME} ${SRC_WIN})

	find_package(SDL2 CONFIG REQUIRED)
	find_package(sdl2-gfx CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_gfx)
endif()

if(UNIX)
	file(GLOB_RECURSE SRC_552
	${SRC_DIR}/src/*.cpp
	${SRC_DIR}/main.cpp
	)

	add_executable (${PROJECT_NAME} ${SRC_552})

	set(SDL2_DIR ${CMAKE_MODULE_PATH} ${SYSROOT}/usr/lib/cmake/SDL2)

	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
endif()

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${TOP_DIR}/${ROMS_DIR}
                ${APP_DIR}/${ROMS_DIR})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${TOP_DIR}/${TEST_DIR}
                ${APP_DIR}/${TEST_DIR})
