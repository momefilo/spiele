cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(spiele_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(spiele
spiele.c
)

add_subdirectory(./include/buttons)
add_subdirectory(./include/tetris)
add_subdirectory(./include/ILI9341)
add_subdirectory(./include/sound)
add_subdirectory(./include/snake)
add_subdirectory(./include/ranking)


pico_add_extra_outputs(spiele)
target_link_libraries(spiele
		snake
		tetris
		ranking
		pico_stdlib
		buttons
		ILI9341
		sound
		)
