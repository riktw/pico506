add_library(lt_logger lt_logger.c)
target_link_libraries(lt_logger PRIVATE pico_time_headers)
target_include_directories(lt_logger PUBLIC .)
