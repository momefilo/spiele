file(GLOB FILES *.c *.h)
add_library(snake ${FILES})

target_link_libraries(snake
	pico_stdlib
	buttons
	melodys
	ranking
	ili9341
	)
target_include_directories(snake PUBLIC ./)
