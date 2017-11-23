/*
	- crc32bench - CRC32 Benchmark
	Copyright (C) 2017 CYRIL INGENIERIE
	Licenses: GPL2

	- Prerequisities -
		C compiler: gcc or clang
		TASKSET: part of the util-linux package

	- Compilation -
		cc -Wall -O0 crc32bench.c -o crc32bench

	- Input -
		# ASM version
		taskset -c 1 ./crc32bench a GenuineIntel AuthenticAMD

		# C version
		taskset -c 1 ./crc32bench c GenuineIntel AuthenticAMD

	- Results (gcc) -
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
*/

#include <stdio.h>
#include <string.h>

#define RDTSC64(_val64)							\
	asm volatile (							\
			"lfence"			"\n\t"		\
			"rdtsc"				"\n\t"		\
			"shlq	$32,	%%rdx"		"\n\t"		\
			"orq	%%rdx,	%%rax"		"\n\t"		\
			"movq	%%rax,	%0"				\
			:"=m" (_val64)					\
			:						\
			:"%rax","%rcx","%rdx","memory"			\
	);

typedef unsigned int (CRC32vFunc)(unsigned char*, unsigned int);

// Source C: Linux/tools/pcmcia/crc32hash.c
unsigned int CRC32vC(unsigned char *data, unsigned int len)
{
	unsigned int rem = 0, oloop = len, iloop;

	while (oloop--) {
		rem ^= *data++;
		for (iloop = 0; iloop < 8; iloop++)
			rem = (rem >> 1) ^ ((rem & 1) ? 0x436f7265 : 0);
	}
	return(rem);
}

// ASM: Optimization
#define CRC32vASM(data, len)						\
({									\
	unsigned int rem = 0;						\
	asm (								\
		"	movl	%[_len], %%r8d"		"\n\t"		\
		"	movq	%[_data], %%r10"	"\n\t"		\
		"	addq	%%r10, %%r8"		"\n\t"		\
		"	movl	$0, %%r12d"		"\n\t"		\
		"	movl	$0x436f7265, %%r9d"	"\n\t"		\
		".LOOP:"				"\n\t"		\
		"	cmpq	%%r8, %%r10"		"\n\t"		\
		"	je	.EXIT"			"\n\t"		\
		"	addq	$1, %%r10"		"\n\t"		\
		"	movzbl	-1(%%r10), %%edx"	"\n\t"		\
		"	xorl	%%edx, %%r12d"		"\n\t"		\
		"	movl	$8, %%edx"		"\n\t"		\
		".INSIDE:"				"\n\t"		\
		"	movl	%%r12d, %%r11d"		"\n\t"		\
		"	shrl	%%r11d"			"\n\t"		\
		"	andl	$1, %%r12d"		"\n\t"		\
		"	cmovne	%%r9d, %%r12d"		"\n\t"		\
		"	xorl	%%r11d, %%r12d"		"\n\t"		\
		"	subl	$1, %%edx"		"\n\t"		\
		"	jne	.INSIDE"		"\n\t"		\
		"	jmp	.LOOP"			"\n\t"		\
		".EXIT:"				"\n\t"		\
		"	movl	%%r12d, %[_rem]"			\
		: [_rem] "+m" (rem)					\
		: [_data] "m" (data),					\
		  [_len] "im" (len)					\
		: "memory", "rdx", "%r8", "%r9", "%r10", "%r11", "%r12"	\
	);								\
	rem;								\
})

unsigned int CRC32vA(unsigned char *data, unsigned int len)
{
	return(CRC32vASM(data, len));
}

void help(char *program) {
	printf("Usage: %s <c|a> <data1> <data2>\n", program);
}

int main(int argc, char *argv[])
{
	CRC32vFunc *CRC32 = NULL;
	unsigned int crc32[2] = {0, 0}, toggle = 0;

	if (argc == 4) {
		volatile unsigned long long start, stop;
		unsigned long long bench = 1LLU << 24;
		unsigned int len[2] = {strlen(argv[2]), strlen(argv[3])};

		switch (argv[1][0]) {
			case 'c':
			case 'C':
				CRC32 = CRC32vC;
				break;
			case 'a':
			case 'A':
				CRC32 = CRC32vA;
				break;
			default:
				help(argv[0]);
				return(2);
		};

		RDTSC64(start);
		while (bench--) {
			unsigned char *data = (unsigned char *)argv[2 + toggle];

			crc32[toggle] = CRC32(data,len[toggle]);

			toggle = !toggle;
		}
		RDTSC64(stop);

		printf("Benchmark = %llu cycles\n", stop - start);
		for (toggle = 0; toggle < 2; toggle++)
			printf("%s(%s,%u) => %x\n",
				argv[0], argv[2 + toggle], len[toggle],
				crc32[toggle]);
		return(0);
	} else {
		help(argv[0]);
		return(1);
	}
}
