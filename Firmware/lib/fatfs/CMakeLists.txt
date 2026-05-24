add_library(
	fatfs
	fatfs/source/ff.c
	fatfs/source/ffsystem.c
	fatfs/source/ffunicode.c
)
target_compile_options(fatfs PRIVATE --include=ffconfig.h)
target_include_directories(fatfs PUBLIC fatfs/source .)
