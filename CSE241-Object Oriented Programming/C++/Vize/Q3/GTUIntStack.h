#ifndef STACK_H
#define STACK_H
#include <iostream>

class GTUIntStack {
public:
    GTUIntStack();
    GTUIntStack(const GTUIntStack& s);
    ~GTUIntStack();
    void push(int n);
    void pop();
    bool isEmpty() const;
    static int getStacksCreated(); //Sen static int getStacksCreated() const; yazmışsın. Static fonksiyonlar const almaz çünkü herhangi bir 
    //obje ile değil doğrudan sınıf ile alakalıdır. Yani const yapılacak obje olmadığı için const yazılmaz
    friend std::ostream& operator<<(std::ostream& out ,const GTUIntStack& s);
    int size() const;
private:
    int *data;
    int size;
    int capacity;
    static int numCreated;
    void resize();
};

std::ostream& operator<<(std::ostream& out ,const GTUIntStack& s);

#endif