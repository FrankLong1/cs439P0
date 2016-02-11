#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "util.h"


// Referenced Solution to Prob. 8.7 on Pg 798
void sigInt_handler(int sig){
	ssize_t bytes;
	const int STDOUT = 1;
	bytes = write(STDOUT, "Nice try.\n", 10);
   		if(bytes != 10)
   			exit(-999);
   	return;
}

void sigUsr1_handler(int sig){
	ssize_t bytes;
	const int STDOUT = 1;
	bytes = write(STDOUT, "exiting\n", 10);
   		if(bytes != 10)
   			exit(-999);
   	exit(1);
   	return;
}

/*
 * First, print out the process ID of this process.
 *
 * Then, set up the signal handler so that ^C causes
 * the program to print "Nice try.\n" and continue looping.
 *
 * Finally, loop forever, printing "Still here\n" once every
 * second.
 */
int main(int argc, char **argv)
{
	pid_t pid = getpid();
	ssize_t bytes; 
	const int STDOUT = 1;
	struct timespec time;
	struct timespec time2;
	time.tv_sec = 1; 

	printf("%ld\n", (long)pid);
	if(Signal(SIGINT, sigInt_handler) == SIG_ERR)
		unix_error("signal error\n");
	if(Signal(SIGUSR1, sigUsr1_handler) == SIG_ERR)
		unix_error("signal error\n");
   	while(1){
   		nanosleep(&time, &time2);
   		bytes = write(STDOUT, "Still here\n", 12);
   		if(bytes != 12)
   			exit(-999);
   	}

  return 0;
}
