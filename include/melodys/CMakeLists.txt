file(GLOB FILES *.c *.h)
add_library(melodys ${FILES})

target_link_libraries(melodys
	pico_stdlib
	pico_multicore
	sound
)
target_include_directories(melodys PUBLIC ../)
