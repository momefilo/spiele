file(GLOB FILES *.c *.h)
add_library(snake ${FILES})
target_link_libraries(snake
	pico_stdlib
	hardware_adc
	)
target_include_directories(snake PUBLIC ../)
