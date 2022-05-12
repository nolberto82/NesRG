# CMakeList.txt : CMake project for NesRG, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set(CMAKE_CXX_FLAGS_DEBUG "/ZI")
	set(CMAKE_CXX_FLAGS_RELEASE "/ZI")
	set(CMAKE_SHARED_LINKER_FLAGS "/SAFESEH:NO")
	set(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message("debug mode")
	add_compile_definitions(DEBUG=1)
else()
	add_compile_definitions(DEBUG=0)
ENDIF()

set(CMAKE_CXX_STANDARD 17)

set (ASSETS_DIR "assets")

set(TOP_DIR ${CMAKE_SOURCE_DIR})
set(PRJ_DIR ${TOP_DIR}/${PROJECT_NAME})
set(APP_DIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${PRJ_DIR}/src/include)
include_directories(${PRJ_DIR}/nes_apu_emu)

file(GLOB_RECURSE EMU_SRC
${PRJ_DIR}/src/apu.cpp
${PRJ_DIR}/src/controls.cpp
${PRJ_DIR}/src/cpu.cpp
${PRJ_DIR}/src/mappers.cpp
${PRJ_DIR}/src/mem.cpp
${PRJ_DIR}/src/ppu.cpp
${PRJ_DIR}/src/sdlgfx.cpp
${PRJ_DIR}/src/mappers/mapper000.cpp
${PRJ_DIR}/src/mappers/mapper001.cpp
${PRJ_DIR}/src/mappers/mapper002.cpp
${PRJ_DIR}/src/mappers/mapper003.cpp
${PRJ_DIR}/src/mappers/mapper004.cpp
${PRJ_DIR}/src/mappers/mapper009.cpp
${PRJ_DIR}/Simple_Apu.cpp
${PRJ_DIR}/Sound_Queue.cpp
${PRJ_DIR}/nes_apu_emu/apu_snapshot.cpp
${PRJ_DIR}/nes_apu_emu/Blip_Buffer.cpp
${PRJ_DIR}/nes_apu_emu/Multi_Buffer.cpp
${PRJ_DIR}/nes_apu_emu/Nes_Apu.cpp
${PRJ_DIR}/nes_apu_emu/Nes_Namco.cpp
${PRJ_DIR}/nes_apu_emu/Nes_Oscs.cpp
${PRJ_DIR}/nes_apu_emu/Nes_Vrc6.cpp
${PRJ_DIR}/nes_apu_emu/Nonlinear_Buffer.cpp
)

if(WIN32)
	file(GLOB_RECURSE WIN_SRC
	${PRJ_DIR}/windows/main.cpp
	${PRJ_DIR}/windows/gui.cpp
	${PRJ_DIR}/windows/breakpoints.cpp
	${PRJ_DIR}/windows/tracer.cpp
	${PRJ_DIR}/imgui/imgui.cpp
	${PRJ_DIR}/imgui/imgui_demo.cpp
	${PRJ_DIR}/imgui/imgui_draw.cpp
	${PRJ_DIR}/imgui/imgui_tables.cpp
	${PRJ_DIR}/imgui/imgui_widgets.cpp
	${PRJ_DIR}/imgui/imgui_impl_sdl.cpp
	${PRJ_DIR}/imgui/imgui_impl_sdlrenderer.cpp
	)

	include_directories(${PRJ_DIR}/windows/include)	
	include_directories(${PRJ_DIR}/imgui)

	add_executable (${PROJECT_NAME} ${WIN_SRC} ${EMU_SRC})

	find_package(SDL2 CONFIG REQUIRED)
	#find_package(sdl2-image CONFIG REQUIRED)
	#find_package(glad CONFIG REQUIRED)
	find_package(sdl2-gfx CONFIG REQUIRED)

	target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_gfx opengl32)
endif()

if(UNIX)
	add_executable (${PROJECT_NAME} ${EMU_SRC})

	set(SDL2_DIR ${CMAKE_MODULE_PATH} ${SYSROOT}/usr/lib/cmake/SDL2)

	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
endif()

if(NOT ${APP_DIR}/${ASSETS_DIR})
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
	            ${TOP_DIR}/${ASSETS_DIR}
                ${APP_DIR}/${ASSETS_DIR})
endif()