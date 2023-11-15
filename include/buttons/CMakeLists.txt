file(GLOB FILES *.c *.h)
add_library(buttons ${FILES})
target_link_libraries(buttons
	pico_stdlib
	hardware_gpio
)
target_include_directories(buttons PUBLIC ../)
