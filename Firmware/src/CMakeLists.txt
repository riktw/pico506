add_executable(
	${PROJECT_NAME}
	clicker.c
	diskio.c
	pico506.c
	st506.c
	storage.c
	utils.c
)

add_subdirectory(../lib/lt_logger lt_logger)
add_subdirectory(../lib/fatfs fatfs)
add_subdirectory(../lib/sdlib sdlib)

target_link_libraries(
	${PROJECT_NAME}
	pico_stdlib
	pico_multicore
	pico_time_headers
	hardware_pio
	hardware_dma
	hardware_spi
	hardware_pwm
	lt_logger
	fatfs
	sdlib
)

# pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/rll_2of7.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/st506_rddt.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/st506_rdgt.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/st506_step.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/st506_wrdt.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/st506_wrgt.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/st506_wrrm.pio)
pico_add_extra_outputs(${PROJECT_NAME})
