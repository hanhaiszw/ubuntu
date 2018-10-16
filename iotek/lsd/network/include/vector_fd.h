#ifndef __VECTOR_H__
#define __VECTOR_H__

typedef struct{
    int *fd;  //socket描述符 指向一个数组 里面存储着所有的fd
    int counter; //当前vecotr中包含的fd个数
    int max_counter; //vecotr的最大容量
}VectorFD;

//创建
extern VectorFD* create_vector_fd(void);
//销毁
extern void destroy_vector_fd(VectorFD *);
//获取指定下标的fd
extern int get_fd(VectorFD *, int index);
//删除指定的fd
extern void remove_fd(VectorFD *, int fd);
//添加指定的fd
extern void add_fd(VectorFD *, int fd);

#endif
