#ifndef PROFILE_H
#define PROFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <kos.h>
#include <time.h>

/***************************************************************************
	Time measurement
***************************************************************************/
/*
	Example of time measurement usage:
	
	pfMicroseconds p = pfStart();
	//Code to measure
	p = pfEnd(p);	//p now contains time elapsed in microseconds
*/

typedef uint64 pfMicroseconds;	//Time in microseconds

/*
	Returns the start time of measurement period
*/
static inline pfMicroseconds pfStart() {
	return timer_us_gettime64();
}

/*
	Returns length of measurement period
*/
static inline pfMicroseconds pfEnd(pfMicroseconds start) {
	pfMicroseconds end = timer_us_gettime64();
	return end-start;
}

float pfSecs(pfMicroseconds time);
float pfMillisecs(pfMicroseconds time);
float pfCycles(pfMicroseconds time);
/*
	Returns average cycles per loop iteration
*/
float pfCycLoop(pfMicroseconds time, int loops);

/***************************************************************************
	Hardware performance counter
***************************************************************************/
#define PM_CR_BASE	((volatile int16_t*)0xff000084)
#define PM_CTR_BASE	((volatile int32_t*)0xff100004)
#define PMCR(n)         (PM_CR_BASE[n*2])
#define PMCTRH(n)       (PM_CTR_BASE[n*2])
#define PMCTRL(n)       (PM_CTR_BASE[n*2+1])

#define PMCR_PMM_MASK   0x0000003f

#define PMCR_CLKF       0x00000100
#define PMCR_PMCLR      0x00002000
#define PMCR_PMST       0x00004000
#define PMCR_PMENABLE   0x00008000

typedef uint64 pfCounter;	//Number of events

typedef enum {
	PE_OC_READ_ACCESS = 0x1,
	PE_OC_WRITE_ACCESS = 0x2,
	PE_OC_RW_ACCESS = 0xe, //?
	PE_OC_READ_MISS = 0x4,
	PE_OC_WRITE_MISS = 0x5,
	PE_OC_ALL_MISS = 0xf,
	PE_OC_ALL_ACCESS = 0x9, //?
	PE_OC_LINE_READ_CYCLES = 0x22,
	
	PE_OCRAM_ACCESS = 0xb,
	PE_CHIPIO_ACCESS = 0xd,
	
	PE_IC_READ_HIT = 0x6,
	PE_IC_READ_MISS = 0x8,
	PE_IC_ALL_ACCESS = 0xa,
	PE_IC_LINE_READ_CYCLES = 0x21,
	
	PE_UTLB_MISS = 0x3,
	PE_ITLB_MISS = 0x7,
	
	PE_BRANCH_COUNT = 0x10,
	PE_BRANCH_TAKEN = 0x11,
	PE_SUBROUTINE_CALLS = 0x12,
	
	PE_SINGLE_INSTRUCTIONS_EXECUTED = 0x13,
	PE_DUAL_INSTRUCTIONS_EXECUTED = 0x14,
	PE_FPU_INSTRUCTIONS_EXECUTED = 0x15,
	
	PE_IRQ_COUNT = 0x16,
	PE_NMI_COUNT = 0x17,
	PE_TRAPA_COUNT = 0x18,
	PE_UBCA_COUNT = 0x19,
	PE_UBCB_COUNT = 0x1a,
	
	PE_CYCLE_COUNT = 0x23,
	PE_STALL_IC_MISS_CYCLES = 0x24,
	PE_STALL_OC_MISS_CYCLES = 0x25,
	PE_STALL_BRANCH_CYCLES = 0x27,
	PE_STALL_CPU_CYCLES = 0x28,
	PE_STALL_FPU_CYCLES = 0x29,
} PFC_PERF_EVENT;

static inline pfCounter pfcRead(int idx) {
	pfCounter result = (pfCounter)(PMCTRH(idx) & 0xffff) << 32;
	result |= PMCTRL(idx);
	return result;
}

static inline void pfcClear(int idx) {
	PMCR(idx) = PMCR(idx) | PMCR_PMCLR;
}

static inline void pfcStop(int idx) {
	PMCR(idx) &= ~(PMCR_PMM_MASK | PMCR_PMENABLE);
}

static inline void pfcStart(int idx, PFC_PERF_EVENT event) {
	pfcStop(idx);
	pfcClear(idx);
	PMCR(idx) = event | PMCR_PMENABLE | PMCR_PMST;
}

/***************************************************************************
	Sampling profiler
***************************************************************************/
/*
	Initalizes sampling profiler to profile the current thread.
	size is the number of samples to allocate for the sample buffer
	Profiler defaults to stopped state with a clear sample buffer.
	
	The profiler runs in the watchdog thread, so the watchdog must be
	initialized before initializing the profiler. If you want to run
	the profiler without the watchdog, initialize the watchdog and call
	wdPause() immediately, and just leave the watchdog paused.
*/
void pfSampInit(size_t size);

/*
	Stops profiler and deallocates sample buffer
*/
void pfSampShutdown(void);

/*
	Stop profiler and clears the sample buffer
*/
void pfSampReset(void);

/*
	Enable profiler
*/
void pfSampGo(void);

/*
	Stop profiler from collecting samples
*/
void pfSampStop(void);

/*
	Returns true if profiler is running, false if profiler is stopped
*/
int pfSampIsRunning(void);

/*
	Saves profile samples to file. 
	
	If append is 0, file is overwritten. If append is 1, samples are appended
	to end of file.
	
	Profiler must not be running when saving samples.
	
	Does not clear sample buffer.
*/
void pfSampSave(const char *fname, int append);

#ifdef __cplusplus
}
#endif

#endif
