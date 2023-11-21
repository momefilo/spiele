file(GLOB FILES *.c *.h)
add_library(ili9341 ${FILES})
target_link_libraries(ili9341
	pico_stdlib
	hardware_spi
	)
target_include_directories(ili9341 PUBLIC ./)
