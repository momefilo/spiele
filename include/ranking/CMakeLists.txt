file(GLOB FILES *.c *.h)
add_library(ranking ${FILES})

target_link_libraries(ranking
	pico_stdlib
	flash
	ili9341
)
target_include_directories(ranking PUBLIC ../)
