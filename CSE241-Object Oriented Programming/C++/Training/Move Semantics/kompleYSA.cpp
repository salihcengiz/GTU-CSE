#include <iostream>
#include <stdexcept> // For std::out_of_range

/* Bu kod eksikYSA.cpp dosyasına şu promptun verilmesi sonucunda oluşturuldu: Complete this class definition below  with all big five functions.
Add some cout statements in all big five functions so that I can trace them. Write a main function to demo the big five uses.
*/

class DynamicIntArray {
    private:
        int* data;           // Pointer to the array's data
        std::size_t size;    // Current number of elements
        std::size_t capacity;// Allocated capacity of the array

        // Function to resize the array when capacity is reached
        void resize(std::size_t newCapacity) {
            std::cout << "Resizing to capacity: " << newCapacity << "\n";
            int* newData = new int[newCapacity];
            for (std::size_t i = 0; i < size; ++i) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }

    public:
        // 1. Constructor
        DynamicIntArray() : data(nullptr), size(0), capacity(0) {
            std::cout << "Default constructor called.\n";
        }

        // 2. Destructor
        ~DynamicIntArray() {
            std::cout << "Destructor called.\n";
            delete[] data;
        }

        // 3. Copy Constructor
        DynamicIntArray(const DynamicIntArray& other) : data(new int[other.capacity]), size(other.size), capacity(other.capacity) {
            std::cout << "Copy constructor called.\n";
            for (std::size_t i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        }

        // 4. Move Constructor
        DynamicIntArray(DynamicIntArray&& other) noexcept : data(other.data), size(other.size), capacity(other.capacity) {
            std::cout << "Move constructor called.\n";
            other.data = nullptr;
            other.size = 0;
            other.capacity = 0;
        }

        // 5. Copy Assignment Operator
        DynamicIntArray& operator=(const DynamicIntArray& other) {
            std::cout << "Copy assignment operator called.\n";
            if (this != &other) {
                delete[] data; // Free existing resources

                size = other.size;
                capacity = other.capacity;
                data = new int[capacity];
                for (std::size_t i = 0; i < size; ++i) {
                    data[i] = other.data[i];
                }
            }
            return *this;
        }

        // 6. Move Assignment Operator
        DynamicIntArray& operator=(DynamicIntArray&& other) noexcept {
            std::cout << "Move assignment operator called.\n";
            if (this != &other) {
                delete[] data; // Free existing resources

                data = other.data;
                size = other.size;
                capacity = other.capacity;

                other.data = nullptr;
                other.size = 0;
                other.capacity = 0;
            }
            return *this;
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
    std::cout << "Creating arr...\n";
    DynamicIntArray arr;
    arr.addElement(10);
    arr.addElement(20);
    arr.addElement(30);

    std::cout << "Creating arr2 as a copy of arr...\n";
    DynamicIntArray arr2 = arr; // Invokes copy constructor
                                                                    //Önemli nokta
    std::cout << "Creating arr3 by moving arr...\n";
    DynamicIntArray arr3 = std::move(arr); // Invokes move constructor

    std::cout << "Contents of arr2 (copy): ";
    for (std::size_t i = 0; i < arr2.getSize(); ++i) {
        std::cout << arr2.getElement(i) << ' ';
    }
    std::cout << "\n";

    std::cout << "Contents of arr3 (moved): ";
    for (std::size_t i = 0; i < arr3.getSize(); ++i) {
        std::cout << arr3.getElement(i) << ' ';
    }
    std::cout << "\n";

    std::cout << "Assigning arr2 to arr3...\n";
    arr3 = arr2; // Invokes copy assignment operator

    std::cout << "Assigning a new DynamicIntArray to arr3 via move...\n";
    arr3 = DynamicIntArray(); // Invokes move assignment operator

    return 0;
}
