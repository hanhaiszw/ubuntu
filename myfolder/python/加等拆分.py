a=[100]   #a是一个列表
def test(num):
    #num+=num   #1
    num=num+num #2
    print(num)

test(a)
print(a)

#1 输出
'''
[100,100]
[100,100]
'''

#2 输出
'''
[100,100]
[100]
'''
