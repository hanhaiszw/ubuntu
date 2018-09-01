a=4
b=5

#第一种
c=a
a=b
b=c

#第二种
a=a+b
b=a-b
a=a-b

#第三种 python独有
a,b=b,a

print("a=%d,b=%d"%(a,b))

