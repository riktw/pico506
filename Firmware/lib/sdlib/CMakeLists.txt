add_library(
	sdlib
	sd.c
	sd_debug.c
	sd_spi.c
)
target_link_libraries(
	sdlib
	PRIVATE
	pico_time_headers
	pico_stdlib
	hardware_spi
	hardware_dma
	lt_logger
)
target_include_directories(sdlib PUBLIC .)
