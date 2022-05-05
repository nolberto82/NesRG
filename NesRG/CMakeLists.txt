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
	add_definitions(-DHAVE_STDINT_H)	
endif()

set(CMAKE_CXX_STANDARD 17)

set (ASSETS_DIR "assets")

set(TOP_DIR ${CMAKE_SOURCE_DIR})
set(SRC_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_NAME})
set(APP_DIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${SRC_DIR}/src/include)
include_directories(${SRC_DIR}/src/mappers/include)
include_directories(${SRC_DIR}/nes_apu_emu)

file(GLOB_RECURSE EMU_SRC
${SRC_DIR}/src/cpu.cpp
${SRC_DIR}/src/mem.cpp
${SRC_DIR}/src/ppu.cpp
${SRC_DIR}/src/apu.cpp
${SRC_DIR}/src/sdlcore.cpp
${SRC_DIR}/src/controls.cpp
${SRC_DIR}/src/mappers/mapper001.cpp
${SRC_DIR}/src/mappers/mapper002.cpp
${SRC_DIR}/src/mappers/mapper004.cpp
${SRC_DIR}/src/mappers/mapper009.cpp
${SRC_DIR}/src/mappers/mapper009.cpp
${SRC_DIR}/Simple_Apu.cpp
${SRC_DIR}/Sound_Queue.cpp
${SRC_DIR}/nes_apu_emu/apu_snapshot.cpp
${SRC_DIR}/nes_apu_emu/Blip_Buffer.cpp
${SRC_DIR}/nes_apu_emu/Multi_Buffer.cpp
${SRC_DIR}/nes_apu_emu/Nes_Apu.cpp
${SRC_DIR}/nes_apu_emu/Nes_Namco.cpp
${SRC_DIR}/nes_apu_emu/Nes_Oscs.cpp
${SRC_DIR}/nes_apu_emu/Nes_Vrc6.cpp
${SRC_DIR}/nes_apu_emu/Nonlinear_Buffer.cpp
)

if(WIN32)
	file(GLOB_RECURSE WIN_SRC
	${SRC_DIR}/windows/main.cpp
	${SRC_DIR}/windows/gui.cpp
	${SRC_DIR}/windows/breakpoints.cpp
	${SRC_DIR}/windows/tracer.cpp
	${SRC_DIR}/imgui/imgui.cpp
	${SRC_DIR}/imgui/imgui_demo.cpp
	${SRC_DIR}/imgui/imgui_draw.cpp
	${SRC_DIR}/imgui/imgui_tables.cpp
	${SRC_DIR}/imgui/imgui_widgets.cpp
	${SRC_DIR}/imgui/imgui_impl_sdl.cpp
	${SRC_DIR}/imgui/imgui_impl_sdlrenderer.cpp
	)

	include_directories(${SRC_DIR}/windows/include)	
	include_directories(${SRC_DIR}/imgui)

	add_executable (${PROJECT_NAME} ${WIN_SRC} ${EMU_SRC})

	find_package(SDL2 CONFIG REQUIRED)
	#find_package(sdl2-image CONFIG REQUIRED)
	find_package(sdl2-gfx CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_gfx)
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