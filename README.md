# crc32bench
CRC32 Benchmark for x86_64 processors

##	- Prerequisities -
		C compiler: gcc or clang
		TASKSET: part of the util-linux package

##	- Compilation -
		cc -Wall -O0 crc32bench.c -o crc32bench

##	- Input -
		# ASM version
		taskset -c 1 ./crc32bench a GenuineIntel AuthenticAMD

		# C version
		taskset -c 1 ./crc32bench c GenuineIntel AuthenticAMD

##	- Results (gcc) -
		# ASM version
		Benchmark = 8882632044 cycles
		crc32bench(GenuineIntel,12) => 75a2ba39
		crc32bench(GenuineIntel,12) => 75a2ba39

		# C version (gcc)
		Benchmark = 31046108769 cycles
		crc32bench(GenuineIntel,12) => 75a2ba39
		crc32bench(AuthenticAMD,12) => 3485bbd3

		# C version (clang)
		Benchmark = 17586801852 cycles
		crc32bench(GenuineIntel,12) => 75a2ba39
		crc32bench(AuthenticAMD,12) => 3485bbd3
