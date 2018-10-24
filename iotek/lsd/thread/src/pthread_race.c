#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//定义线程运行函数
void* th_fn(void* arg){
    //int distance=*((int*)arg); //会报Segment fault,非法访问内存错误
    int distance = (int)arg;
    int i;
    for(i=1; i<distance; i++){
        printf("%lx run %d\n",pthread_self(), i);
        int time = 500000;
        usleep(time);
    }

    return (void*)0;
}

int main(void){
    int err;
    pthread_t rabbit,turtle; //定义线程标识符

    //创建rabbit线程
    if((err = pthread_create(&rabbit, NULL, th_fn, (void*)60)) != 0){
        perror("pthread_create error");
    }   

    //创建turtle线程
    if((err = pthread_create(&turtle, NULL, th_fn, (void*)60)) != 0){
        perror("pthread_create error");
    }   

    pthread_join(rabbit, NULL);
    pthread_join(turtle, NULL);
    //输出主控线程的id  这里使用十六进制值输出
    printf("control thread id: %lx\n", pthread_self());
    printf("finished!\n");
    return 0;
}
