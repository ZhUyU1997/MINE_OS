/*
 *    exit_code:	high byte	low byte
 *    	normal exit	  status	   0
 *    	signal exit	    0  		 signal
 *    	job control	  signal	  0177
 */

#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/types.h>

#define	WNOHANG		0x01	/* Not hang if no status, return immediately. */
#define WUNTRACED	0x02	/* Report status of stopped child process.    */

#define WIFEXITED(s)	(!((s)&0xFF))			/* normal exit 	*/
#define WIFSIGNALED(s)	(((unsigned int)(s)-1)&xFFFF) 	/* signal exit 	*/
#define WIFSTOPPED(s)	(((s)&0xFF)==0x7F)		/* job control	*/
#define WEXITSTATUS(s)	(((s) >> 8) & 0377)		/* exit status 	*/
#define WTERMSIG(s)	((s)&0x7F)			/* sig value 	*/
#define WSTOPSIG(s)	(((s)>>8)&0xFF)			/* stop signal 	*/

pid_t wait(pid_t pid, int *status, int options);

#endif
