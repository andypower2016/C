#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#define print(format, args...) printf("[%s:%d]" format "\n", __FUNCTION__, __LINE__, ##args)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct arg 
{
  int id;
};

struct data
{
  int id;
};

// circular queue
struct queue
{
   struct data *queue_data[256];
   int front;
   int rear;
   int size;
} g_queue = {
  .front = -1,
  .rear = -1,
  .size = 5,
};

int push_data(struct data *d)
{
   if(isFull())
   {
     printf("Queue is full\n");
     return 0;
   }
   if(g_queue.front == -1)
   {
      g_queue.front = 0;
   }
   g_queue.rear = (g_queue.rear+1) % g_queue.size;
   g_queue.queue_data[g_queue.rear] = d;
   return 1;
}
int pop_data(struct data **d)
{
  if(isEmpty())
  {
     printf("Queue is empty\n");
     return 0;
  }
  *d = g_queue.queue_data[g_queue.front];
  if(g_queue.front == g_queue.rear) // if last element is poped
  {
     g_queue.front = g_queue.rear = -1;
  }
  else
  {
     g_queue.front = (g_queue.front + 1) % g_queue.size;
  }
  return 1;
}

int isEmpty()
{
   return (g_queue.front == -1) ? 1 : 0;
}
int isFull()
{
  return ((g_queue.front == 0) && (g_queue.rear == g_queue.size -1)) || ((g_queue.rear+1) == g_queue.front);
}

void queue_work_data(struct data* d)
{
  if(d)
  {
     print("push data");
     pthread_mutex_lock(&mutex);    
     int ret = push_data(d);
     if(!ret)
     {
        while(!ret) // retry
        {
          pthread_cond_signal(&cond);
          pthread_cond_wait(&cond, &mutex); 
          ret = push_data(d);
        }
        pthread_mutex_unlock(&mutex);
     }
     else
     {
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond); 
     }
  } 
}

typedef void(*callback)(struct data* d) ;


int done = 0;
void *run(void* fptr)
{
  callback worker = ((void*)(struct data*) fptr);
  int i = 0;
  while(i < 10)
  {
    // every 2 second gets data
    struct data *d = (struct data*) malloc(sizeof(struct data));
    d->id = ++i;
    worker(d);
    sleep(0.5);
  }
  done = 1;
  return 0;
}

void *printdata(void* arg)
{
  while(!(done == 1 && isEmpty())) 
  {
    pthread_mutex_lock(&mutex);
    if(isEmpty())
    {
      print("waiting for push");
      pthread_cond_wait(&cond, &mutex);
    }  
    print("proceed");
    struct data *d = NULL;
    if(pop_data(&d))
    {
      pthread_mutex_unlock(&mutex);
      pthread_cond_signal(&cond); // signal push
    }
    else
    {
      pthread_mutex_unlock(&mutex);
    }
    
    if(d)
    {
        print("data id = %d", d->id);
        sleep(2); // process 2 sec
        free(d);
    }
    else
    {
      print("d is null");
    }   
  }
  return 0;
}

int main(int argc, char** argv)
{
    pthread_t t1, t2;
    pthread_create(&t1,0,run,(void*) &queue_work_data);
    pthread_create(&t2,0,printdata,NULL);
    pthread_join(t1, 0);
    pthread_join(t2, 0);
    return 0;
}
