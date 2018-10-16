#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include "vector_fd.h"

//只在此文件中使用  可以声明为static  ???
static void encapacity(VectorFD *vfd){
    if(vfd->counter >= vfd->max_counter){
        //realloc 该怎么做
        int *fds = (int*)calloc(vfd->counter+5, sizeof(int));
        assert(fds != NULL);
        memcpy(fds, vfd->fd, sizeof(int) * vfd->counter);
        free(vfd->fd);
        vfd->fd = fds;
        vfd->max_counter += 5;
    }

}

static int indexof(VectorFD *vfd, int fd){
    int i=0;
    for(; i<vfd->counter;i++){
        if(vfd->fd[i] == fd) return i;
    }
    //不存在返回-1
    return -1;

}

//创建
VectorFD* create_vector_fd(void){
    VectorFD *vfd = (VectorFD*)calloc(1,sizeof(VectorFD));
    assert(vfd != NULL);
    //刚开始初始化5个fd空间
    vfd->fd = (int*)calloc(5, sizeof(int));
    assert( vfd-> fd != NULL);
    vfd->counter = 0;
    vfd->max_counter = 5;
    return vfd;
}

//销毁
void destroy_vector_fd(VectorFD *vfd){
    assert(vfd != NULL);
    free(vfd->fd);
    free(vfd);
}
//获取指定下标的fd
int get_fd(VectorFD *vfd, int index){
    assert(vfd != NULL);
    if(index < 0 || index > vfd -> counter-1)
        return 0;  //是不是返回-1更合适些
    return vfd->fd[index];
}
//删除指定的fd
void remove_fd(VectorFD *vfd, int fd){
    assert(vfd != NULL);
    int index = indexof(vfd, fd);
    if(index == -1) return;
    int i = index;
    //从后向前覆盖
    for(;i<vfd->counter-1;i++){
        vfd->fd[i]=vfd->fd[i+1];
    }
    vfd->counter--;
}
//添加指定的fd
void add_fd(VectorFD *vfd, int fd){
    assert(vfd != NULL);
    encapacity(vfd);//扩张动态数组
    vfd->fd[vfd->counter++] = fd;

}




