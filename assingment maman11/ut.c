#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "ut.h"

ut_slot_t *threads_table;
tid_t thread_index;
int threads_table_size;

/* Set by the signal handler. */
static volatile int currThreadNum;


int ut_init(int tab_size){
    //validate table size
    if(tab_size > MAX_TAB_SIZE || tab_size < MIN_TAB_SIZE)
        tab_size = MAX_TAB_SIZE;

    tab_size++;// Save one more slut for the main context
    threads_table = (ut_slot_t*)malloc(tab_size * sizeof(ut_slot_t));
    if(!threads_table)
        return SYS_ERR;

    threads_table_size = tab_size;
    thread_index = 0;
    return 0;
}

tid_t ut_spawn_thread(void (*func)(int), int arg){
    if(thread_index == threads_table_size)
        return TAB_FULL;

   ut_slot_t t;
   //allocate stack size for the new thread
   ucontext_t uct, mainc;
   char *st = (char*)malloc(STACKSIZE);
   //check st is valid
   if(!st)
       return SYS_ERR;

   //set context
   if(getcontext(&uct) == -1)
        return SYS_ERR;

   //set context definition
   uct.uc_link = &mainc;
   uct.uc_stack.ss_sp = st;
   uct.uc_stack.ss_size =  STACKSIZE;
   makecontext(&uct, (void(*)(void))func, 1, arg);

   //Set thread arguments
   t.uc = uct;
   t.arg = arg;
   t.vtime = 0;
   t.func = func;

    threads_table[thread_index] = t;
    //insert thread to thread table

    return thread_index++;
}

/* This is the signal handler which swaps between the threads. */
void handler(int signal) {

    if (signal == SIGVTALRM) {
        threads_table[currThreadNum].vtime += 100;
    }
    else {
    	alarm(1);//set the quantom
        //Swap between threads
        int numOfThreads = thread_index;
        int previousThread = currThreadNum % numOfThreads;
    	currThreadNum = ((currThreadNum + 1) % numOfThreads) ;
        threads_table[currThreadNum].vtime += 100;
    	if(swapcontext(&((threads_table[previousThread]).uc),&((threads_table[currThreadNum]).uc)) == -1)
        {
            perror("Error: swithcing between threads");
            exit(1);
        }
    }
}

int ut_start(void){
    struct sigaction sa;
    struct itimerval itv;
    ucontext_t mainc;

    sa.sa_flags = SA_RESTART;
	sigfillset(&sa.sa_mask);
	sa.sa_handler = handler;

    /* set up vtimer for accounting */
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 100000;
    itv.it_value = itv.it_interval;

	/* Install the signal handler and vtimer. */
	if (sigaction(SIGALRM, &sa, NULL) < 0 || sigaction(SIGVTALRM, &sa, NULL) < 0
            || setitimer(ITIMER_VIRTUAL, &itv, NULL) < 0)
		return SYS_ERR;

    //start running the first thread
    alarm(1);
    currThreadNum = 0;
    ut_slot_t s = threads_table[currThreadNum];
    if(swapcontext(&mainc, &s.uc) == -1)
        return SYS_ERR;

    return 0;
}

unsigned long ut_get_vtime(tid_t tid){
    ut_slot_t s = threads_table[tid];
    return (s.vtime);
}
