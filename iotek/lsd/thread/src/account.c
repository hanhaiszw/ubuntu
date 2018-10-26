#include "account.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
//创建账户
Account* create_account(int code, double balance){
    Account* a=(Account*)malloc(sizeof(Account));
    assert(a!=NULL);
    a->code = code;
    a->balance=balance;
    //对互斥锁进行初始化
    pthread_mutex_init(&a->mutex, NULL);
    return a;
}

//销毁账户
void destroy_account(Account* a){
    assert(a!=NULL);
    //销毁互斥锁
    pthread_mutex_destroy(&a->mutex);
    free(a);
}

//取款
double withdraw(Account *a, double amt){
    assert(a!=NULL);
    //上锁  上锁与加锁之间的代码称为临界区
    pthread_mutex_lock(&a->mutex);  //对共享资源进行加锁

    if(amt < 0 || amt > a->balance){
        //释放互斥锁
        pthread_mutex_unlock(&a->mutex);
        return 0.0;
    }
    double balance = a->balance;
    sleep(1); 
    balance -= amt;
    a->balance = balance;


    //释放互斥锁
    pthread_mutex_unlock(&a->mutex);
    return amt;
}

//存款
double deposit(Account *a, double amt){
    assert(a!=NULL);
    //上锁  上锁与加锁之间的代码称为临界区
    pthread_mutex_lock(&a->mutex);  //对共享资源进行加锁
    if(amt < 0){
        //释放互斥锁
        pthread_mutex_unlock(&a->mutex);
        return 0.0;
    }
    double balance = a->balance;
    sleep(1);
    balance += amt;
    a->balance = balance;
    //释放互斥锁
    pthread_mutex_unlock(&a->mutex);
    return amt;
}

//查看账户余额
double get_balance(Account *a){
    assert(a!=NULL);
    //上锁  上锁与加锁之间的代码称为临界区
    pthread_mutex_lock(&a->mutex);  //对共享资源进行加锁
    double balance = a->balance;
    //释放互斥锁
    pthread_mutex_unlock(&a->mutex);
    return balance;
}




