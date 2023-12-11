file(GLOB FILES *.c *.h)
add_library(klotski ${FILES})

target_link_libraries(klotski
	pico_stdlib
	buttons
	melodys
	flash
	ili9341
	hardware_flash
)
target_include_directories(klotski PUBLIC ./)
