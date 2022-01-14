#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#define CLOCKID CLOCK_REALTIME
#define SIGNAL_NUM 5

#define print(format, args...) printf(format "\n", ##args)

char time_info[256];

void* setup_timer(void *arg);
void timer_handler(int signal);
char* get_current_time();

void* setup_timer(void *arg)
{
    timer_t timer;
    struct sigevent signal_event;
    struct itimerspec timer_spec;

    /* setup signal event */
    signal_event.sigev_notify = SIGEV_SIGNAL;
    signal_event.sigev_signo = SIGALRM;
    signal_event.sigev_value.sival_ptr = &timer;
    signal(signal_event.sigev_signo, timer_handler);
    
    /* create timer */  
    if(timer_create(CLOCKID, &signal_event, &timer) == -1)
        exit(-1);

    /* setup timer spec */
    timer_spec.it_value.tv_sec = 1;  /* timer start delay time */
    timer_spec.it_value.tv_nsec = 0;
    timer_spec.it_interval.tv_sec = 2; /* time spacing */
    timer_spec.it_interval.tv_nsec = 0;

    /* start timer */ 
    print("[%s] timer start", get_current_time());
    if (timer_settime(timer, 0, &timer_spec, NULL) == -1)
         exit(-1);

    return 0;
}

void timer_handler(int signal)
{
    print("[%s] captured signal=%d", get_current_time(), signal);
}

char* get_current_time()
{
    struct tm *cursystem = NULL;
    time_t cur;
    time(&cur);
    cursystem = localtime(&cur);
    if(cursystem) {
        sprintf(time_info, "%02d:%02d:%02d", 
            cursystem->tm_hour, cursystem->tm_min, cursystem->tm_sec);
    }
    else {
        sprintf(time_info, "error occured");
    }
    return &time_info;
}

void *run(void* arg)
{
    while(1)
    {
        int left = sleep(5);
        print("[%s] left=%d", get_current_time(), left);
    }
}

int main(int argc, char *argv[])
{
    pthread_t thread1;
    pthread_create(&thread1, 0, setup_timer, NULL);

    pthread_t thread2;
    pthread_create(&thread2, 0, run, NULL);
      
    pthread_join(thread1, 0);
    pthread_join(thread2, 0);
    return 0;
}
