/* 
 * msh - A mini shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "util.h"
#include "jobs.h"


/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "msh> ";    /* command line prompt (DO NOT CHANGE) */
static struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
void usage(void);
void sigquit_handler(int sig);



/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{ 
    printf("evaluating ");
    int pid;
    int validCmd;
    char *argv[MAXARGS];
    int bg = parseline(cmdline, argv);
    if(argv[0] == NULL) return;
    if(!builtin_cmd(argv)) {
        //Prevents race condition referenced page 779
        //sigset_t mask_all, mask_one, prev_one;
        sigset_t mask_one;
        //sigfillset(&mask_all);
        sigemptyset(&mask_one);
        sigaddset(&mask_one, SIGCHLD);
        sigprocmask(SIG_BLOCK, &mask_one, NULL);

        //Something is wrong with the way that we're unblocking
        pid = fork();
        if(pid == 0){
            //addjob(jobs, pid, FG, cmdline);
            
            sigprocmask(SIG_UNBLOCK, &mask_one, NULL);
            setpgid(0, 0);
            validCmd = execve(argv[0], argv, environ);
            if(validCmd < 0){
                printf("%s: Is not a valid command\n", argv[0]);
                exit(0);
            }
        } 
        else {
            //printf("%s\n", cmdline);
            if(bg) {
                //printf("IM IN THE BACKGROUND\n");
                //listjobs(jobs);
                addjob(jobs, pid, BG, cmdline);
                sigprocmask(SIG_UNBLOCK, &mask_one, NULL);
                printf("[%d] (%d) %s", pid2jid(jobs, pid), pid, cmdline);
                //after they're added, unblock in multiple places
                //print out job when running in bg
            }
            else {
                //printf("IM IN THE BACKGROUND\n");
                addjob(jobs, pid, FG, cmdline);
                sigprocmask(SIG_UNBLOCK, &mask_one, NULL);
                waitfg(pid);
            }
        }
    }
}


/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 * Return 1 if a builtin command was executed; return 0
 * if the argument passed in is *not* a builtin command.
 */
int builtin_cmd(char **argv) 
{
    printf("builtin ");
    if(strcmp(argv[0], "quit") == 0) {
        exit(0);
    }
    if(strcmp(argv[0], "jobs") == 0) {
        listjobs(jobs);
        return 1;
    }
    if(strcmp(argv[0], "&") == 0){
        return 1;
    }    
    if(strcmp(argv[0], "bg") == 0) {
        do_bgfg(argv);
        return 1;
    }    
    if(strcmp(argv[0], "fg") == 0) {
        do_bgfg(argv);
        return 1;
    }
    
    return 0;     /* not a builtin command */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
    int id;
    struct job_t *j;
    if(argv[1][0] == '%') {
        char * a = argv[1];
        a++;
        id = atoi(a);
        j = getjobjid(jobs, id);
    } 
    else {
        id = atoi(argv[1]);
        j = getjobpid(jobs, id);
    }

    kill(j->pid, SIGCONT);
    if(strcmp(argv[0], "bg") == 0) {
        j->state = BG;
    }    
    if(strcmp(argv[0], "fg") == 0) {
        j->state = FG;
        waitfg(id);
    }
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
 // Referenced Pg 782
void waitfg(pid_t pid)
{
    sigset_t mask;
    sigset_t prev;
    //sigemptyset(&prev);
    //sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);

    sigprocmask(SIG_BLOCK, &mask, NULL);
        while(pid != fgpid(jobs)){
        sigsuspend(&prev);
    }
    //deletejob(jobs, fpid);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    //pid_t fpid = fgpid(jobs);
    pid_t pid;
    int status;
    char message[41];
    ssize_t bytes;
    const int STDOUT = 1;
    //sprintf(message, "before while");
           // bytes = write(STDOUT, message, 41); 
           // if(bytes != 41) exit(-999);       
    while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0){
            //sprintf(message, "in while");
            //bytes = write(STDOUT, message, 41); 
           // if(bytes != 41) exit(-999);   
        // sprintf(message, "Child [%d] terminated with exit status = %d\n", pid, WEXITSTATUS(status));
        // bytes = write(STDOUT, message, 50); 
        // if(bytes != 50) exit(-999);
        //

        if(WIFEXITED(status)) {
            sprintf(message, "Job [%d] (%d) terminated by signal 2", pid2jid(jobs, pid), pid);
            bytes = write(STDOUT, message, 41); 
            if(bytes != 41) exit(-999);
            deletejob(jobs, pid);                
        }
        else if (WIFSTOPPED(status) && sig == SIGINT) {
            sprintf(message, "Job [%d] (%d) stopped by signal 20", pid2jid(jobs, pid), pid);
            bytes = write(STDOUT, message, 41); 
            if(bytes != 41) exit(-999);
            struct job_t *j = getjobpid(jobs, pid);
            //redundant?
            j->state = ST;  
        } else if (WIFSIGNALED(status)) {
            sprintf(message, "Job [%d] (%d) terminated by signal %d", pid2jid(jobs, pid), pid, sig);
            bytes = write(STDOUT, message, 41); 
            if(bytes != 41) exit(-999);
            deletejob(jobs, pid);    
        }
        else {
            // sprintf(message, "in else %d", pid);
            // bytes = write(STDOUT, message, 41); 
            // if(bytes != 41) exit(-999); 
            // deletejob(jobs, pid);
        }
    }
    return;
}
// probably move state change and delete

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    pid_t fpid = fgpid(jobs);

    if(fpid) {
        // ssize_t bytes;
        // const int STDOUT = 1;
        // char message[41];
        // sprintf(message, "Job [%d] (%d) terminated by signal 2", pid2jid(jobs, fpid), fpid);
        // bytes = write(STDOUT, message, 41); 
        // if(bytes != 41) exit(-999);
        deletejob(jobs, fpid);
        kill(-fpid, SIGINT);
    }
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    pid_t fpid = fgpid(jobs);

    if(fpid) {
        //struct job_t *j;
        // ssize_t bytes;
        // const int STDOUT = 1;
        // char message[41];
        // sprintf(message, "Job [%d] (%d) stopped by signal 20", pid2jid(jobs, fpid), fpid);
        // bytes = write(STDOUT, message, 41); 
        // if(bytes != 41) exit(-999);
        // do in sig child instead
        //deletejob(jobs, fpid);
        //j = getjobpid(jobs, fpid);
        //j->state = ST;
        kill(-fpid, SIGTSTP);
    }
    return;
}

/*********************
 * End signal handlers
 *********************/



/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    ssize_t bytes;
    const int STDOUT = 1;
    bytes = write(STDOUT, "Terminating after receipt of SIGQUIT signal\n", 45);
    if(bytes != 45)
       exit(-999);
    exit(1);
}



