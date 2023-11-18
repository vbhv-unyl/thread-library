#include "speed.h"

#define STACK_SIZE 4096
static int currentThread = -1;
static int threadCount = 0;
static int running = 0;
static TCB threadList[MAX];

static void wrapperfn();
void alarm_handler(int signo);

#ifdef __x86_64__

typedef unsigned long address_t;
#define SP 6
#define PC 7

address_t translate_address(address_t address)
{
    address_t return_address;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
                 : "=g"(return_address)
                 : "0"(address));
    return return_address;
}

#else

typedef unsigned int address_t;
#define SP 4
#define PC 5

address_t translate_address(address_t address)
{
    address_t return_address;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
                 : "=g"(return_address)
                 : "0"(address));
    return return_address;
}

#endif

static void wrapperfn()
{
    if (threadList[currentThread].type == ARG)
    {
        threadList[currentThread].ret_val = threadList[currentThread].f2(threadList[currentThread].args);
    }
    else
    {
        threadList[currentThread].f1();
    }

    printf("Thread %d exited\n", getID());
    deleteThread(currentThread);
    signal(SIGALRM, alarm_handler);
    alarm(1);
    dispatch(SIGALRM);
}

void alarm_handler(int signo)
{
    signal(SIGALRM, alarm_handler);
    dispatch(SIGALRM);
}

void initialize(INFO *stats, int id)
{
    assert(stats != NULL);
    stats->ID = id;
    stats->state = DEAD;
    stats->total_execution_time = 0;
    stats->total_sleep_time = 0;
    stats->total_ready_time = 0;
    stats->ready_time_start = 0;
    stats->run_time_start = 0;
}

void terminate(void)
{
    int count = 0;
    for (int i = (currentThread + 1) % MAX; count < MAX; i = (i + 1) % MAX)
    {
        if (threadList[i].stats.state != DEAD)
        {
            threadList[i].stats.avg_wait_time = threadList[i].stats.total_ready_time / threadList[i].stats.burst;
            threadList[i].stats.avg_time_quanta = threadList[i].stats.total_execution_time / threadList[i].stats.burst;
            printf("Thread ID: %d\nTotal Execution Time: %d\nTotal Sleep Time: %d\nTotal bursts: %d\nAverage Waiting Time: %f\nAverage Time Quanta: %f\n\n\n\n\n", i, threadList[i].stats.total_execution_time, threadList[i].stats.total_sleep_time, threadList[i].stats.burst, threadList[i].stats.avg_wait_time, threadList[i].stats.avg_time_quanta);
            threadList[i].stats.state = DEAD;
            count++;
        }
    }
    exit(0);
}

int create(void (*f)(void))
{
    int ID = -1;
    if (running == 0)
    {
        for (int i = 0; i < MAX; i++)
            initialize(&(threadList[i].stats), i);

        running = 1;
    }

    for (int i = 0; i < MAX; i++)
    {
        if (threadList[i].stats.state == DEAD)
        {
            ID = i;
            break;
        }
    }

    if (ID == -1)
    {
        fprintf(stderr, "ID allocation failed\n");
        return id;
    }

    assert(ID >= 0 && ID < MAX);
    threadList[id].type = NOARG;
    threadList[id].f1 = f;
    threadList[id].stats.state = NEW;
    threadCount += 1;
    return ID;
}

int createArgs(void *(*f)(void *), void *arg)
{
    int ID = -1;
    if (running == 0)
    {
        for (int i = 0; i < MAX; i++)
            initialize(&(threadList[i].stats), i);

        running = 1;
    }

    for (int i = 0; i < MAX; i++)
    {
        if (threadList[i].stats.state == DEAD)
        {
            ID = i;
            break;
        }
    }

    if (ID == -1)
    {
        fprintf(stderr, "ID allocation failed\n");
        return id;
    }

    threadList[id].type = ARG;
    threadList[id].f2 = f;
    threadList[id].args = arg;
    threadList[id].stats.state = NEW;
    threadCount += 1;
    return ID;
}

int getID()
{
    return currentThread;
}

void dispatch(int signo)
{
    int count = 0;
    signal(SIGALRM, SIG_IGN);

    if (threadList[currentThread].stats.state != DEAD)
    {
        int return_val = sigsetjmp(threadList[currentThread].buffer, 1);
        if (return_val == 1)
            return;

        threadList[currentThread].stats.state = READY;
        threadList[currentThread].stats.total_execution_time += (clock() - threadList[currentThread].stats.run_time_start) / CLOCKS_PER_SEC;
        threadList[currentThread].stats.ready_time_start = clock();
    }

    for (int i = (currentThread + 1) % MAX; count <= MAX; i = (i + 1) % MAX)
    {
        if (threadList[i].stats.state == READY)
        {
            currentThread = i;
            break;
        }
        count++;
    }

    if (count > MAX)
        exit(0);

    threadList[currentThread].stats.state = RUN;
    threadList[currentThread].stats.total_ready_time += (clock() - threadList[currentThread].stats.ready_time_start) / CLOCKS_PER_SEC;
    threadList[currentThread].stats.run_time_start = clock();
    threadList[currentThread].stats.burst += 1;
    signal(SIGALRM, alarm_handler);
    alarm(1);
    siglongjmp(threadList[currentThread].buffer, 1);
}

void yield()
{
    dispatch(SIGALRM);
}

void run(int threadID)
{
    threadList[threadID].stats.state = READY;
}

void resume(int threadID)
{
    threadList[threadID].stats.state = READY;
}

void suspend(int threadID)
{
    threadList[threadID].stats.state = SUSPEND;
}

void sleep(int sec)
{
    threadList[currentThread].stats.state = SLEEP;
    clock_t st = clock();

    while (((clock() - st) / CLOCKS_PER_SEC) <= sec)
        ;
    threadList[currentThread].stats.total_sleep_time += sec;

    dispatch(SIGALRM);
}

void deleteThread(int threadID)
{
    threadCount -= 1;
    threadList[threadID].stats.state = DEAD;
    printf("Thread ID = %d has been deleted!\n", threadID);
}

INFO *getStatus(int threadID)
{
    INFO *res = &(threadList[threadID].stats);
    return res;
}

void start()
{
    address_t stack_pointer, program_counter;
    for (int i = 0; i < MAX; i++)
    {
        sp = (address_t)threadList[i].stack + STACK_SIZE - sizeof(address_t);
        pc = (address_t)wrapperfn;
        sigsetjmp(threadList[i].buffer, 1);
        (threadList[i].buffer)[SP] = translate_address(stack_pointer);
        (threadList[i].buffer)[PC] = translate_address(program_counter);
        if (threadList[i].stats.state == NEW)
            threadList[i].stats.state = READY;
    }

    signal(SIGALRM, alarm_handler);
    alarm(1);
    while (1)
        ;
}
