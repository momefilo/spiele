file(GLOB FILES *.c *.h)
add_library(ILI9341 ${FILES})
target_link_libraries(ILI9341
	pico_stdlib
	hardware_spi
	)
target_include_directories(ILI9341 PUBLIC ../)
