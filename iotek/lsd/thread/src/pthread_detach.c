#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//输出下属性值
void out_state(pthread_attr_t *attr){
    int state;
    if(pthread_attr_getdetachstate(attr, &state) != 0){
        perror("getdetachstate error");
    }else{
        if(state == PTHREAD_CREATE_JOINABLE){
            printf("joinable state\n");
        }else if(state ==  PTHREAD_CREATE_DETACHED){
            printf("detached state\n");
        }else{
            printf("error state\n");
        }    
    }
}

void* th_fn(void *arg){
    int i=0;
    int sum=0;
    for(i=0; i<=100;i++){
        sum+=i;
    }
    return (void*)sum;
}

int  main(void){
    int err;
    pthread_t default_th, detach_th;

    //定义线程属性
    pthread_attr_t attr;
    //初始化线程属性
    pthread_attr_init(&attr);
    //输出分离属性
    out_state(&attr);
    
    if((err = pthread_create(&default_th, &attr, th_fn, (void*)0))!=0){
        perror("pthread create error");
    }
    int res;
    if((err = pthread_join(default_th, (void*)&res))!=0){
        perror("pthread join error");
    }else{
        printf("default return is %d\n", (int)res);
    }
    
    printf("-----------------------------------------------\n");
    //设置分离属性 分离状态启动线程
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    out_state(&attr);
    if((err = pthread_create(&detach_th, &attr, th_fn, (void*)0)) != 0){
        perror("pthread create error");
    }
    //对一个设置分离属性的线程再调用pthread_join会出错
    //在这里获取不到线程的返回值
    if((err = pthread_join(detach_th, (void*)&res)) != 0){
        //执行这里
        //perror("pthread join error");
        //会输出无效的参数
        fprintf(stderr, "%s\n", strerror(err));  //错误编码
    }else{
        printf("default return is %d\n", (int)res);
    }

    //销毁线程属性
    pthread_attr_destroy(&attr);
    printf("0x%lx finished\n", pthread_self());
    
    sleep(1);
    return 0;
    
}
