cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
set(PICO_BOARD yd_rp2040)
project(spiele_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(spiele
spiele.c
)


add_subdirectory(./snake)
add_subdirectory(./tetris)
add_subdirectory(./klotski)
add_subdirectory(./include/libs/ili9341)
add_subdirectory(./include/libs/flash)
add_subdirectory(./include/libs/sound)
add_subdirectory(./include/buttons)
add_subdirectory(./include/melodys)
add_subdirectory(./include/ranking)


pico_add_extra_outputs(spiele)
target_link_libraries(spiele
		snake
		tetris
		klotski
		buttons
		ranking
		melodys
		ili9341
		flash
		sound
		pico_stdlib
		)
