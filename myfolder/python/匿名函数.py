def test(a,b,func):
    result = func(a,b)
    return result

# 测试输入值为   lambda x,y:x+y
func_new = input("请输入一个匿名函数：") #获取到的是一个字符串
func_new = eval(func_new)   # 转化为lambda表达式
num = test(11,22,func_new)
print(num)   # 输出 33
