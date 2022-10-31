#include <assert.h>
#include "profile.h"
#include "watchdog.h"

#define CLOCKS_PER_SECOND	(200000000.0f)

float pfSecs(pfMicroseconds time)
{
	return time / 1000000.0f;
}

float pfMillisecs(pfMicroseconds time)
{
	return time / 1000.0f;
}

float pfCycles(pfMicroseconds time)
{
	return CLOCKS_PER_SECOND * pfSecs(time);
}

float pfCycLoop(pfMicroseconds time, int loops)
{
	return pfCycles(time) / loops;
}



typedef struct {
	unsigned char addr[3];
} small_address_t;

static kthread_t *profile_thread = NULL;	//thread being profiled
static volatile int profile_inited = 0;
static volatile int profiling = 0;

static small_address_t *profile_store = NULL;
static int profile_store_pos = 0;
static int profile_store_size = 0;

void pfSampInit(size_t size)
{
	assert(wdIsInited());
	assert(!profile_inited);

	profile_inited = 1;
	
	profile_thread = thd_current;
	profile_store_size = size;
	assert(profile_store == NULL);
	profile_store = malloc(sizeof(small_address_t) * profile_store_size);
	
	pfSampReset();
}

void pfSampReset(void)
{
	assert(profile_inited);
	pfSampStop();
	profile_store_pos = 0;
}

void pfSampShutdown(void)
{
	assert(profile_inited);
	
	profile_thread = NULL;
	profile_inited = 0;
	profiling = 0;
	if (profile_store)
		free(profile_store);
	profile_store = NULL;
	profile_store_pos = 0;
	profile_store_size = 0;
}

void pfSampGo(void)
{
	assert(profile_inited);
	assert(!profiling);
	profiling = 1;
}

void pfSampStop(void)
{
	assert(profile_inited);
	profiling = 0;
}

int pfSampIsRunning(void)
{
	return profiling;
}

void pfSampRecordSample(void)
{
	if (profile_store_pos >= profile_store_size) {
		pfSampStop();
		printf("Profile records filled!\n");
		fflush(stdout);
	} else {
		unsigned int pc = (unsigned int)profile_thread->context.pc;
		
		profile_store[profile_store_pos].addr[0] = pc & 0xff;
		profile_store[profile_store_pos].addr[1] = (pc>>8) & 0xff;
		profile_store[profile_store_pos].addr[2] = (pc>>16) & 0xff;
		profile_store_pos++;
	}
}

void pfSampSave(const char *fname, int append)
{
	assert(profile_inited);
	assert(!profiling);
	
	wdPause();
	
	printf("Writing %i profile samples to \"%s\"... ", profile_store_pos,fname);
	fflush(stdout);
	
	FILE *of;
	size_t written = 0;
	
	of = fopen(fname, append ? "a" : "w");
	if (of != NULL) {
		int i;
		for(i = 0; i < profile_store_pos; i++) {
			uint32 addr = (profile_store[i].addr[2] << 16) | (profile_store[i].addr[1] << 8) | profile_store[i].addr[0];
			written += fprintf(of,"8C%06X\n",addr);
		}
		fclose(of);
		printf("Done! %i bytes written (ferror: %i)\n",written, ferror(of));
	} else {
		printf("Unable to write samples (ferror: %i)\n", ferror(of));
	}
	
	fflush(stdout);
	
	wdResume();
}
