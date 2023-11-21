file(GLOB FILES *.c *.h)
add_library(flash ${FILES})
target_link_libraries(flash
	pico_stdlib
	hardware_flash
	hardware_sync
)
target_include_directories(flash PUBLIC ./)
