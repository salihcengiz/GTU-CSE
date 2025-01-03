#include <iostream>
#include <stdexcept>
#include <vector>
#include <list>
#include <forward_list>

using namespace std;

template <class Container, class T>
T* findInContainer(const vector<T>& vec, T value){
    T *p;
    for(p = vec.begin(); p != vec.end(); ++p){
        if(p* == value)
            return p;
    }
    throw runtime_error("Value not found");
}

int main() {
    vector<int> myIntVector = {1, 2, 3, 4, 5};
    vector<double> myDoubleVector = {1.3, 2.7, 3.8, 4.9, 5.2};
    vector<char> myCharVector = {'A', 'B', 'C', 'D', 'E'};

    list<int> myIntList = {1, 2, 3, 4, 5};
    list<double> myDoubleList = {1.3, 2.7, 3.8, 4.9, 5.2};
    list<char> myCharList = {'A', 'B', 'C', 'D', 'E'};

    forward_list<int> myIntForwardList = {1, 2, 3, 4, 5};
    forward_list<double> myDoubleForwardList = {1.3, 2.7, 3.8, 4.9, 5.2};
    forward_list<char> myCharForwardList = {'A', 'B', 'C', 'D', 'E'};

    try{
        cout << "Value is: " << findInContainer(myIntVector, 4);
        cout << "Value is: " << findInContainer(myDoubleVector, 1.3);
        cout << "Value is: " << findInContainer(myCharVector, 'A');

        cout << "Value is: " << findInContainer(myIntList, 4);
        cout << "Value is: " << findInContainer(myDoubleList, 1.3);
        cout << "Value is: " << findInContainer(myCharList, 'A');

        cout << "Value is: " << findInContainer(myIntForwardList, 4);
        cout << "Value is: " << findInContainer(myDoubleForwardList, 1.3);
        cout << "Value is: " << findInContainer(myCharForwardList, 'A');
    }
    catch(runtime_error& e){
        cout << "Error: " << e.what() << endl;
    }
    return 0;
}