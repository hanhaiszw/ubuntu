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
        int time = (int)(drand48() * 100000);
        usleep(time);
    }

    return (void*)distance;
}

int main(void){
    int err;
    pthread_t rabbit,turtle; //定义线程标识符

    //创建rabbit线程
    if((err = pthread_create(&rabbit, NULL, th_fn, (void*)60)) != 0){
        perror("pthread_create error");
    }   

    //创建turtle线程
    if((err = pthread_create(&turtle, NULL, th_fn, (void*)50)) != 0){
        perror("pthread_create error");
    }   

    //主控线程调用pthread_join()自己会阻塞
    //直到rabbit和turtle线程运行结束方可运行
    //调用pthread_join 线程执行完成后,线程占有的资源会自动释放
    int result;
    pthread_join(rabbit, (void*)&result);
    printf("rabbit's distance is %d\n",result);
    pthread_join(turtle, (void*)&result);
    printf("turtle's distance is %d\n",result);
    //输出主控线程的id  这里使用十六进制值输出
    printf("control thread id: %lx\n", pthread_self());
    printf("finished!\n");
    return 0;
}
