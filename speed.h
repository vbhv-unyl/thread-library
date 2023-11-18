#ifndef SPEED_H
#define SPEED_H
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<assert.h>
#include<setjmp.h>
#include<ctype.h>
#include<time.h>
#include<unistd.h>
#define MAX 100
typedef void (*fnargs) (void);
typedef void* (*fargs) (void*);
enum STATES {DEAD = 1, RUN, READY, SLEEP, SUSPEND, NEW};
enum TYPE {ARG = 1, NOARG};

typedef struct INFO{
    int ID;
    int total_ready_time, total_sleep_time, total_execution_time, burst;
    float avg_wait_time, avg_time_quanta;
    clock_t ready_time_start, run_time_start;
    enum STATES state;
} INFO;

typedef struct TCB{
    enum TYPE type;
    INFO stats;
    jmp_buf buffer;
    char stack[4096];
    fnargs f1;
    fargs f2;
    void *args, *ret_val;
} TCB;

int create(void (*f) (void));
int createArgs(void* (*f) (void*), void* args);
int getID(void);
void dispatch(int);
void start(void);
void run(int threadID);
void suspend(int threadID);
void resume(int threadID);
void yield(void);
void sleep(int sec);
void deleteThread(int threadID);
void initialize(INFO *stat, int id);
INFO* getStatus(int threadID);
void terminate(void); // STOP the master thread

#endif