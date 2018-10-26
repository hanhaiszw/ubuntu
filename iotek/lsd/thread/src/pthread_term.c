#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

//获取线程返回值
typedef struct
{
    int d1;
    int d2;
}Arg;

void* th_fn(void* arg)
{
    Arg* r =(Arg*)arg;
    // return (void*)(r->d1 + r->d2);
    return (void*)r;
}

int main(void){
    int err;
    pthread_t th;
    Arg r={10, 50};
    if((err = pthread_create(&th, NULL, th_fn, (void*)&r)) != 0){
        perror("pthread_create error");
    }


    //获取线程函数返回结果

    //接受一个整型值
    //1
    /*
    int *result;
    pthread_join(th, (void**)&result);
    printf("result is %d\n", (int)result);
    */

    //2
    //int result;
    //pthread_join(th,(void*)&result);
    //printf("result is %d\n",result);

    //接受一个返回的结构体
    int *result;
    pthread_join(th, (void**)&result);
    printf("result is %d\n", ((Arg*)result) -> d1 + ((Arg*)result) -> d2);
    return 0;
}
