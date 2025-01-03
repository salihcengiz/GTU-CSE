#include <iostream>
#include <vector>

using namespace std;

int main() {

    //Aşağıdaki örnekler için gerekli deklerasyonlar
    std::vector<int> other_vec;
    int index, value;

    //Size Constructor: Creates a vector with a specified number of default-initialized elements.
    // Vector with 10 elements, each initialized to 0
    std::vector<int> myVec(10);
     
    //Range Constructor: Creates a vector from a range of iterators.
    std::vector<int> vec(other_vec.begin(), other_vec.end());

    //Copy Constructor: Creates a copy of another vector.
    std::vector<int> vec(other_vec);

    //operator[]: Accesses an element without bounds checking.
    vec[index] = value;

    //at(): Accesses an element with bounds checking; throws std::out_of_range if the index is invalid.
    vec.at(index) = value;

    //front(): Returns a reference to the first element. Undefined if vector is empty.
    int& first = vec.front();

    //back(): Returns a reference to the last element. Undefined if vector is empty.
    int& last = vec.back();

    //data(): Returns a pointer to the underlying array.
    int* ptr = vec.data();

    //size(): Returns the number of elements.
    size_t s = vec.size();

    //empty(): Checks if the vector is empty.
    bool is_empty = vec.empty();

    //capacity(): Returns the total allocated storage capacity of the vector.
    size_t cap = vec.capacity();

    //reserve(): Increases the capacity to a specified amount if necessary.
    vec.reserve(100);

    //resize(): Changes the vector’s size. If the new size is larger, default values are appended.
    vec.resize(20);

    //insert(): Inserts elements at a specified position.
    vec.insert(vec.begin() + index, value);

    //erase(): Removes elements at a specified position or range.
    vec.erase(vec.begin() + index);

    //clear(): Removes all elements.
    vec.clear();


    //Iterators
    //begin() and end(): Returns iterators to the beginning and end of the vector. 
    auto it1 = vec.begin();
    auto it2 = vec.end();

    //rbegin() and rend(): Returns reverse iterators.
    auto rit1 = vec.rbegin();
    auto rit2 = vec.rend();



    return 0;
}