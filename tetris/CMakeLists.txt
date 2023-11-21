file(GLOB FILES *.c *.h)
add_library(tetris ${FILES})

target_link_libraries(tetris
	pico_stdlib
	hardware_adc
	buttons
	melodys
	ranking
	ili9341
)
target_include_directories(tetris PUBLIC ./)
