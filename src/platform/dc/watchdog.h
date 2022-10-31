#ifndef WATCHDOG_H
#define WATCHDOG_H

#ifdef __cplusplus
extern "C" {
#endif

/*
	Initializes watchdog thread. Watchdog defaults to running.
	
	Watchdog will call panic() and cause program to quit/return to dcload
	on timeout. This avoids having to manually reboot the system if
	the program locks up.
	
	Periodically call wdPet() to prevent timeout (once per frame is good).
*/
void wdInit();

/*
	Resets watchdog timer
	Call periodically (e.g. once per frame) to stop watchdog from activating
*/
void wdPet();

/*
	Pauses watchdog timer
	
	Pause the timer when it isn't possible to pet the dog, like when
	preforming a large read from disc or dcload.
	
	It is safe to nest calls to wdPause and wdResume. If multiple calls to
	wdPause are made, the watchdog will only unpause when a matching number
	or wdResume calls are made.
	
	So
		wdPause();
		wdPause();
		wdResume();
		wdPause();
		wdResume();
		wdResume();
	...will unpause only at the final wdResume call.
*/
void wdPause();

/*
	Unpauses watchdog timer
*/
void wdResume();

/*
	Returns true if watchdog timer is running
*/
int wdIsRunning();

/*
	Returns true if watchdog timer is has been initialized
*/
int wdIsInited();

#ifdef __cplusplus
}
#endif


#endif

