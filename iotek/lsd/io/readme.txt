测试IO系统调用
把/etc目录下的passwd复制到当前目录
bin/cp /etc/passwd ./passwd
bin/cp为自己编写的调用系统调用的复制程序

//先编译io.c 再编译cp.c
//编译为.o文件
gcc -o obj/io.o Iinclude -c src/io.c
//编译链接
gcc -o bin/cp Iinclude obj/io.o src/cp.c
