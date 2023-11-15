file(GLOB FILES *.c *.h)
add_library(ranking ${FILES})
target_link_libraries(ranking
	pico_stdlib
	hardware_flash
	hardware_sync
	)
target_include_directories(ranking PUBLIC ../)
