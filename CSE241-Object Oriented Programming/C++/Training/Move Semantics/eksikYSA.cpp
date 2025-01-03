#include <iostream>
#include <stdexcept> // For std::out_of_range

class DynamicIntArray {
    private:
        int* data;            // Pointer to the array's data
        std::size_t size;     // Current number of elements
        std::size_t capacity; // Allocated capacity of the array

        // Function to resize the array when capacity is reached
        void resize(std::size_t newCapacity) {
            int* newData = new int[newCapacity];
            for (std::size_t i = 0; i < size; ++i) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }
    public:
        // Constructor to initialize the dynamic array
        DynamicIntArray()
            : data(nullptr), size(0), capacity(0) {}

        // Destructor to free allocated memory
        ~DynamicIntArray() {
            if(data != nullptr)
            delete[] data;
        }
        
        DynamicIntArray(const DynamicIntArray& o): size(o.size), capacity(o.capacity) {
            data = new int[size];
            for(int i = 0; i < size; ++i)
            data[i] = o.data[i];
        }
        
        DynamicIntArray(DynamicIntArray&& o) noexcept: size(o.size), capacity(o.capacity), data(o.data) { 
            o.data = nullptr;
            o.size = 0;
            o.capacity = 0;
        }
        
        DynamicIntArray& DynamicIntArray::operator=(DynamicIntArray && o) {
            if( this != &o){
                delete[] data;
                size = o.size;
                capacity = o.capacity;
                data = o.data;
                
                o.data = nullptr;
                o.size = 0;
                o.capacity = 0;
            }
        }

        // Function to add an element to the array
        void addElement(int element) {
            if (size == capacity) {
                // If capacity is zero, initialize it to 1; otherwise, double it
                std::size_t newCapacity = (capacity == 0) ? 1 : capacity * 2;
                resize(newCapacity);
            }
            data[size++] = element;
        }

        // Function to get the element at a specific index
        int getElement(std::size_t index) const {
            if (index >= size) {
                throw std::out_of_range("Index out of range");
            }
            return data[index];
        }

        // Function to get the current size of the array
        std::size_t getSize() const {
            return size;
        }
};

int main() {
    DynamicIntArray arr;
    arr.addElement(10);
    arr.addElement(20);
    arr.addElement(30);
	
    DynamicIntArray arr2(std::move(arr));  // !!! dont do that, so dangerous
	
    for (std::size_t i = 0; i < arr.getSize(); ++i) {
        std::cout << arr.getElement(i) << ' ';
    }
    // Output: 10 20 30

    return 0;
}