// Linux kernel posix timer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#define CLOCKID CLOCK_REALTIME
#define SIGNAL_NUM 5

#define print(format, args...) printf(format "\n", ##args)

char time_info[256];

void* setup_timer(void *arg);
void timer_handler(int signal);
char* get_current_time();

typedef struct timer_setting {
    int interval; // time spacing
    int delay;    // delay before timer start
    int signal;   // signal index
    void *data;   
} timer_setting;

void* setup_timer(void *arg)
{
    timer_setting* ts_data = (timer_setting*) arg;
    if(!ts_data) {
        print("no timer setting data");
        return NULL;
    }
    
    timer_t timer;
    struct sigevent signal_event;
    struct itimerspec timer_spec;

    /* setup signal event */
    signal_event.sigev_notify = SIGEV_SIGNAL;
    signal_event.sigev_signo = ts.signal;
    signal_event.sigev_value.sival_ptr = &timer;
    signal(signal_event.sigev_signo, timer_handler);
    
    /* create timer */  
    if(timer_create(CLOCKID, &signal_event, &timer) == -1) {
        print("[%s:%d] timer_create fail", __FUNCTION__, __LINE__);
        exit(-1);
    }
    /* setup timer spec */
    timer_spec.it_value.tv_sec = ts_data.delay;  /* timer start delay time */
    timer_spec.it_value.tv_nsec = 0;
    timer_spec.it_interval.tv_sec = ts_data.interval; /* time spacing */
    timer_spec.it_interval.tv_nsec = 0;

    /* start timer */ 
    print("[%s] timer start", get_current_time());
    if (timer_settime(timer, 0, &timer_spec, ts.data) == -1) {
          print("[%s:%d] timer_settime fail", __FUNCTION__, __LINE__);
         exit(-1);
    }
    return 0;
}

void timer_handler(int signal, void *data)
{
    print("[%s] captured signal=%d , data=%s", get_current_time(), signal, (char*) data);
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
    return time_info;
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
    timer_setting ts;
    
    // set timer setting
    ts.interval = 2;
    ts.delay = 1;
    ts.signal = SIGALRM;
    ts.data = (char*) malloc(sizeof(char)*256);
    strcpy(ts.data, "abcd");
    ts.data[4] = '\0';
    
    pthread_t thread1;
    pthread_create(&thread1, 0, setup_timer, (void*) &ts);

    pthread_t thread2;
    pthread_create(&thread2, 0, run, NULL);
      
    pthread_join(thread1, 0);
    pthread_join(thread2, 0);
    return 0;
}
