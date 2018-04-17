//********************************************
// this file implements simple binary samaphore library
// Name:  Shlomi Haver
// ID: 204096648
//********************************************
#include "binsem.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

void binsem_init(sem_t *s, int init_val){
    //Set default
    if(init_val != 0)
        init_val = 1;
    // atomicly set the default value of the semaphore
    xchg(s, init_val);
}

void binsem_up(sem_t *s){
    if(*s == 1){
        //wake up the sleeping process (switch to the other process)
        if(raise(SIGALRM))
        {
            perror("Error: cannot signle SIGALRM");
            exit(1);
        }
        return;
    }
    //set the semaphore up
    xchg(s,1);
}

int binsem_down(sem_t *s){
    if(*s == 0){
        //signal that the current process could be switched
        if(raise(SIGALRM))
            return -1;
    } else {
        //set the semaphore down
        xchg(s,0);
    }
    return 0;
}
