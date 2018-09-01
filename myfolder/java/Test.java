//类测试文件
public class Test {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
        if(args.length!=0){
            System.out.println(args[0]);
        }
		Animal animal = new Dog();
		animal.eat();

		Animal animal2 = new Animal();
		animal2.eat();

		Father father = new Son("son");
		father.say();
	}

}
