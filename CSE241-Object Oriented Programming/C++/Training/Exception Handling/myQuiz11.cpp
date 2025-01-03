#include <vector> 
#include <list>         //Bunları unuttun
#include <forward_list> //Bunları unuttun
#include <stdexcept> 
#include <iostream>

using namespace std;

template <class T>
T* findInContainer(vector<T>& vec, T value){
    T* p;
    for(p = vec.begin(); p != vec.end(); ++p){
    if (p* == value)
        return p;
    }
    throw runtime_error("Value not found");
}

int main(){
    vector<int> myVec;
    myVec.add(1);
    myVec.add(2);
    myVec.add(3);

    int myValve = 3;

    try{
        cout<< "The value is: "
            <<findInContainer(myVec, myValve)<<endl;
    }
    catch (runtime_error& e){
        cout<<"Error: "<<e.what()<<endl;
    }
    list<double> myList;
    double myValue2;

    try{
    cout<< "The value is:"
        <<findInContainer(myList, myValue2)<<endl;
    }
    catch (runtime_error& m){
        cout<<" Error: "<<m.what()<<endl;
    }

    forward_list<char> myFL;

    return 0;
}