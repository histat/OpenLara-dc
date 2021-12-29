#ifndef SH4_SQ_H
#define SH4_SQ_H

#include "sh4.h"

/*
	 /--------------------------------- Not part of physical address space
	 |               /----------------- Physical address
	/-\/---------------------------\
	xxxqqqppppppppppppppppppppssssss
	   \-/\------------------/|\---/
	    |           |         |  \----- Offset in store queue
	    |           |         \-------- Store queue 0/1
	    |           \------------------ Offset from start of SQ area	 
	    \------------------------------ QACR value
	 
*/

/*
	Accessing the QACR registers through a struct like this allows the
	compiler to generate this code:
		mov.l	QACRAddr, r0
		mov.l	r2, @r0
		mov.l	r3, @(4, r0)
		
	Rather than this code, if we #defined the individual registers:
		mov.l	QACR0Addr, r0
		mov.l	QACR1Addr, r1
		mov.l	r2, @r0
		mov.l	r3, @r1
*/
struct SH_QACR_LAYOUT {
	unsigned int qacr0, qacr1;
};
#define SH_QACR (*(volatile struct SH_QACR_LAYOUT *)(0xFF000038))
#define SH_SQ_AREA	(0xE0000000)

static inline void sqSetTargetAddress(void *dst) {
	unsigned int highbits = (((unsigned int)dst) >> 24) & 0x1c;
	SH_QACR.qacr1 = SH_QACR.qacr0 = highbits;
}

static inline void * sqGetWritePointer(void *dst) {
	unsigned int inttarg = (((unsigned int)dst) & 0x3FFFFFF) | SH_SQ_AREA;
	return (void*)inttarg;
}

static inline void * sqPrepare(void *dst) {
	sqSetTargetAddress(dst);
	return sqGetWritePointer(dst);
}

static inline void _sqSubmit32Offset(void *sq, int ofs) {
	unsigned int p = (unsigned int)sq;
	sh4PrefetchMem((void*)(p+ofs));
}

static inline void sqSubmit32(void *sq) {
	_sqSubmit32Offset(sq, 0);
}

static inline void sqSubmit64A(void *sq) {
	_sqSubmit32Offset(sq, 0);
}
static inline void sqSubmit64B(void *sq) {
	_sqSubmit32Offset(sq, 32);
}

static inline void * sqSubmit32Advance(void *sq) {
	_sqSubmit32Offset(sq, 0);
	return (void *)((int)sq + 32);
}

static inline void * sqSubmit64BAdvance(void *sq) {
	_sqSubmit32Offset(sq, 32);
	return (void *)((int)sq + 64);
}

/* 
	Copies 32 bytes into the store queue and submits it, then
	returns sq + 32
*/
static inline void * sqCopy32(void *sq, const void *src) {
	int *sqint = (int *)sq;
	const int *srcint = (const int *)src;
	
	*sqint++ = *srcint++;
	*sqint++ = *srcint++;
	*sqint++ = *srcint++;
	*sqint++ = *srcint++;
	*sqint++ = *srcint++;
	*sqint++ = *srcint++;
	*sqint++ = *srcint++;
	*sqint++ = *srcint++;
	
	sqSubmit32(sq);
	return sqint;
}

#endif
