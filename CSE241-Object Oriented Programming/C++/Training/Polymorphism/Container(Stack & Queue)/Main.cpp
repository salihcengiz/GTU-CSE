#include<list>
#include<vector>
 
 /*Container is an abstract class with no function implementations. Stack is a First-In-First-Out class, Queue is a Last-In-First-Out class. 
 STL list class has functions push_back, pop_back, push_front, and pop_front.
 
Implement all classes in a single file. In the same file, write a global function that that takes a vector<Container*>. The function removes all 
the elements in the Containers. Write a main function that calls this global function with Stack and Queue objects.*/

class Container{
	public:
	// default constructor is OK
	    virtual void add(int) = 0;
		virtual int remove() = 0;
		virtual bool empty() const = 0;
	protected:
		std::list<int> data;
};
 
void removeEverything(std::vector<Container *> & v){
	for(int i = 0; i < v.size(); ++i){
		Container * ve = v[i];
		while(!ve->empty())
			ve->remove();
	}
}
 
// FIFO
class A : public Container{
	public:
	    A(): Container(){}
		void add(int i) { data.push_back(i);}
		int remove() {return data.pop_front();}
		bool empty() const { return data.size() == 0;}
};
 
 
// LIFO
class B : public Container{
	public:
	    B(): Container(){}
		void add(int i) { data.push_back(i);}
		int remove() {return data.pop_back();}
		bool empty() const { return data.size() == 0;}
};
 
 
int main(){
	A a1, a2;
	B b1, b2;
	a1.add(20);
	b1.add(20);
	a2.add(30);
	a1.add(40);
	b2.add(50);
	b1.add(60);

	//Container c; => This line is error

	std::vector<Container*> vc;
	vc.push_back(&a1);
	vc.push_back(&a2);
	vc.push_back(&b1);
	vc.push_back(&b2);
 
	removeEverything(vc);

}