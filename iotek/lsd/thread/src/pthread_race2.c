#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
    char name[20];
    int time;
    int start;
    int end;

}RaceArg;

void* th_fn(void* arg){
    RaceArg *r = (RaceArg*)arg;
    int i = r->start;
    for(; i <= r->end; i++){
        printf("%s(%lx) running %d\n", r->name, pthread_self(),i);
        usleep(r->time);
    }
    //pthread_exit((void*)0); //终止线程,返回0
    //return (void*)0;
    return (void*)(r->end - r->start);
}

int main(void){
    int err;
    pthread_t rabbit, turtle;
    RaceArg r_a = {"rabbit", (int)(drand48()*100000000), 20,50};
    RaceArg t_a = {"turtle", (int)(drand48()*100000000), 10,60};

    if((err = pthread_create(&rabbit, NULL, th_fn, (void*)&r_a))!=0){
        perror("pthread_create error");
    }
    if((err = pthread_create(&turtle, NULL, th_fn, (void*)&t_a))!=0){
        perror("pthread_create error");
    }

    int result;
    pthread_join(rabbit, (void*)&result);//第二个参数用来接收线程函数的返回值
    printf("rabbit distance is %d\n", result);
    pthread_join(turtle, (void*)&result);
    printf("turtle distance is %d\n", result);

    printf("control thread id: %lx\n", pthread_self());
    printf("finished\n");
    return 0;
    
}
