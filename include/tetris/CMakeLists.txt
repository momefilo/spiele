file(GLOB FILES *.c *.h)
add_library(tetris ${FILES})
target_link_libraries(tetris
	pico_stdlib
	hardware_adc
)
target_include_directories(tetris PUBLIC ../)
