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
set (ASSETS_DIR "assets")

set(TOP_DIR ${CMAKE_SOURCE_DIR})
set(SRC_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})
set(APP_DIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${SRC_DIR}/include)

file(GLOB_RECURSE EMU_SRC
${SRC_DIR}/src/cpu.cpp
${SRC_DIR}/src/mem.cpp
${SRC_DIR}/src/ppu.cpp
${SRC_DIR}/src/sdlgfx.cpp)

if(WIN32)
	file(GLOB_RECURSE WIN_SRC
	${SRC_DIR}/windows/debugger.cpp
	${SRC_DIR}/windows/breakpoints.cpp
	${SRC_DIR}/imgui/imgui.cpp
	${SRC_DIR}/imgui/imgui_demo.cpp
	${SRC_DIR}/imgui/imgui_draw.cpp
	${SRC_DIR}/imgui/imgui_tables.cpp
	${SRC_DIR}/imgui/imgui_widgets.cpp
	${SRC_DIR}/imgui/imgui_impl_sdl.cpp
	${SRC_DIR}/imgui/imgui_impl_sdlrenderer.cpp
	${SRC_DIR}/main.cpp
	)

	include_directories(${SRC_DIR}/windows)	
	include_directories(${SRC_DIR}/imgui)
	include_directories(${SRC_DIR}/sdl2/include)
	link_directories(${SRC_DIR}/sdl2/lib/x64)	
	add_subdirectory(${SRC_DIR}/nativefiledialog)

	add_executable (${PROJECT_NAME} ${WIN_SRC} ${EMU_SRC})

	#find_package(SDL2 CONFIG REQUIRED)
	#find_package(sdl2-image CONFIG REQUIRED)
	#find_package(sdl2-gfx CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 SDL2main nfd)# SDL2_image SDL2::SDL2_gfx)

	add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${SRC_DIR}/sdl2/lib/x64/SDL2.dll
                ${APP_DIR})
endif()

if(UNIX)
	add_executable (${PROJECT_NAME} ${EMU_SRC})

	set(SDL2_DIR ${CMAKE_MODULE_PATH} ${SYSROOT}/usr/lib/cmake/SDL2)

	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
endif()

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
	            ${TOP_DIR}/${ASSETS_DIR}
                ${APP_DIR}/${ASSETS_DIR})

#add_custom_command(
#        TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#                ${TOP_DIR}/${ROMS_DIR}
#                ${APP_DIR}/${ROMS_DIR})

#add_custom_command(
#        TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#                ${TOP_DIR}/${TEST_DIR}
#                ${APP_DIR}/${TEST_DIR})