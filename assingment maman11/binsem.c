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
    xchg(s, init_val);
}

void binsem_up(sem_t *s){
    if(*s == 1){
        if(kill(getpid(), SIGALRM))
        {
            perror("Error: ");
            exit(1);
        }
        return;
    }
    xchg(s,0);
}

int binsem_down(sem_t *s){
    if(*s == 0){
        if(kill(getpid(), SIGALRM))
            return -1;
    }else{
        xchg(s,1);
    }
    return 0;
}
