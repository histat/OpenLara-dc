#include <kos.h>
#include <assert.h>
#include "profile.h"

//Save profiler sample every __ milliseconds
#define PROFILER_SAMPLE_RATE_MS	(10)

//Panic after __ milliseconds
#define WATCHDOG_TIME_MS	(4000)

static void * watchdog(void *v);

static volatile int sleeptime = 0;
static volatile int paused = 0;
static kthread_t * dogthd = NULL;
static kthread_t * parentthd = NULL;

#define watchdoginitialized() (dogthd != NULL)

void wdInit() {
	assert(dogthd == NULL);
	parentthd = thd_current;
	dogthd = thd_create(0, &watchdog, NULL);
	thd_set_prio(dogthd, 5);
}

int wdIsInited() {
	return dogthd != NULL;
}

void wdPet() {
	assert(wdIsInited());
	
	sleeptime = 0;
}

int wdIsRunning() {
	return wdIsInited() && !paused;
}

void wdPause() {
	assert(wdIsInited());
	
	paused++;
}

void wdResume() {
	assert(wdIsInited());
	
	paused--;
	if (paused < 0) {
		paused = 0;
		printf("Too many watchdog resumes!\n");
		fflush(stdout);
	}
}
#if 0
//I don't think there's really any reason to need to kill the watchdog thread,
//so this function is probably unnecessary
//TODO How does killing the watchdog affect the profiler?
void wdPutDown() {
	assert(wdIsInited());
	
	thd_destroy(dogthd);	//TODO is this the right way to kill a thread in KOS?
	sleeptime = 0;
	paused = 0;
	dogthd = NULL;
	parentthd = NULL;
}
#endif

static void * watchdog(void *v) {
	(void)v;
	do {
		thd_sleep(PROFILER_SAMPLE_RATE_MS);
//		printf("W");
//		fflush(stdout);
		
		extern void pfSampRecordSample(void);
		if (pfSampIsRunning())
			pfSampRecordSample();
		
		if (!paused) {
			sleeptime += PROFILER_SAMPLE_RATE_MS;

			if (sleeptime > WATCHDOG_TIME_MS/2) {
				/* If you want to try some preventative measure before the watchdog activates,
				   like saving data or stopping a script interpreter, do so here.
				*/
			}
			
			if (sleeptime > WATCHDOG_TIME_MS) {
				printf("At PC: %08X, PR: %08X\n",(unsigned int)parentthd->context.pc,(unsigned int)parentthd->context.pr);
				fflush(stdout);
				arch_panic("Watchdog timeout\n");
			}
		}
	} while (1);
}

