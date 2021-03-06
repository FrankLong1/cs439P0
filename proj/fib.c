#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

const int MAX = 13;

static void doFib(int n, int doPrint);


/*
 * unix_error - unix-style error routine.
 */
inline static void 
unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}


int main(int argc, char **argv)
{
  int arg;
  int print;

  if(argc != 2){
    fprintf(stderr, "Usage: fib <num>\n");
    exit(-1);
  }

  if(argc >= 3){
    print = 1;
  }

  arg = atoi(argv[1]);
  if(arg < 0 || arg > MAX){
    fprintf(stderr, "number must be between 0 and %d\n", MAX);
    exit(-1);
  }

  doFib(arg, 1);

  return 0;
}

/* 
 * Recursively compute the specified number. If print is
 * true, print it. Otherwise, provide it to my parent process.
 *
 * NOTE: The solution must be recursive and it must fork
 * a new child for each call. Each process should call
 * doFib() exactly once.
 */
static void 
doFib(int n, int doPrint)
{
  int status;
  int val;
  pid_t pid;

  if(n <= 2) {
    if(doPrint){
      printf("1\n");
    }
    exit(1);
  }

  pid = fork();
  if(pid == 0){
    doFib(n - 1, 0);
  } 
  wait(&status);
  val = WEXITSTATUS(status);

  pid = fork();
  if(pid == 0){
    doFib(n - 2, 0);
  }
  wait(&status); 
  val = val + WEXITSTATUS(status);

  if(doPrint){
    printf("%d\n", val);
  } else {
    exit(val);
  }
}


