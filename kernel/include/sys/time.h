#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <sys/types.h>
#include <sys/param.h>

#define CLOCKS_PER_SEC 	HZ
#define CLK_TCK 	HZ

#define isleap(y)	(((y)%4==0&&(y)%100!=0)||(y)%400==0)

struct timeval {
	time_t tv_sec; 	/* Seconds	*/
	long   tv_usec; /* Microseconds	*/
};

struct timespec {
	time_t tv_sec; 	/* Seconds	*/
	long   tv_nsec; /* Nanoseconds	*/
};

#endif
