

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define Ncook 2 
#define Noven 15
#define Ndeliverer 10
#define Npacker 2
#define Norderlow 1
#define Norderhigh 5
#define Torderlow 1
#define Torderhigh 3
#define Tpaymentlow 1 
#define Tpaymenthigh 3
#define Tprep 1
#define Tbake 10
#define Tpack 1
#define Tdellow 5
#define Tdelhigh 15
#define Cplain 10 
#define Cspecial 12
#define Pfail 0.1
#define Pplain 0.6

void* order(void *threadid);
void rc_check(int rc);
void mutex_destroyer(pthread_mutex_t *thread);
void cond_destroyer(pthread_cond_t *thread);
void mutex_lock(pthread_mutex_t *thread);
void mutex_unlock(pthread_mutex_t *thread);
int initializations();
void destructor();
