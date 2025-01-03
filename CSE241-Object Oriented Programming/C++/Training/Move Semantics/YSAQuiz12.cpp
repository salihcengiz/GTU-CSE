#include <iostream>
#include <memory>


class Set { //Big five kullanıldı
    private:
        // Uses smart pointer to manage a dynamically allocated array for storing elements
        std::unique_ptr<int[]> data; //Dikkat: <int> değil <int[]> kullanıldı. Farkını öğren
        int size;
        int capacity;
    public:
        // Default constructor
        Set(int cap = 10) : size(0), capacity(cap) {
            data = std::make_unique<int[]>(capacity); //Dikkat: yine <int> değil <int[]> kullanıldı. Farkını öğren
        }

        // Destructor
        ~Set() = default; // Burada

        // Copy Constructor
        Set(const Set& other) : size(other.size), capacity(other.capacity) {
            data = std::make_unique<int[]>(capacity);
            std::copy(other.data.get(), other.data.get() + other.size, data.get()); // Detaylarına bak
            /*std::copy(other.data.get(), other.data.get() + other.size, data.get()); yerine şu da yapılabilirdi
            for (int i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
            */
        }

        // Move Constructor
        Set(Set&& other) noexcept : size(other.size), capacity(other.capacity), data(std::move(other.data)) {
            other.size = 0;
            other.capacity = 0;
        }

        /* GPT Önerisi(Farkına Bak):
        // Move Constructor
        Set(Set&& other) noexcept : size(other.size), capacity(other.capacity), data(other.data) {
            other.data = nullptr;
            other.size = 0;
            other.capacity = 0;
        }
        */

        // Copy Assignment Operator
        Set& operator=(const Set& other) {
            if (this == &other) 
                return *this; // Handle self-assignment

            capacity = other.capacity;
            size = other.size;
            data = std::make_unique<int[]>(other.capacity);
            std::copy(other.data.get(), other.data.get() + other.size, data.get());
            /*std::copy(other.data.get(), other.data.get() + other.size, data.get()); yerine şu da yapılabilirdi
            for (int i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
            */

            return *this;
        }

        // Move Assignment Operator
        Set& operator=(Set&& other) noexcept {
            if (this == &other) 
                return *this; // Handle self-assignment
            
            capacity = other.capacity;
            size = other.size;
            data = std::move(other.data);

            other.size = 0;
            other.capacity = 0;
            
            return *this;
        }

        // addElement fonksiyonu
        void addElement(int value) {
            if (doesContain(value)) {
                return; //Ignore duplicates
            }
            if (size >= capacity) {
                //Increase capacity and allocate new array
                capacity *= 2;
                std::unique_ptr<int[]> newData = std::make_unique<int[]>(capacity);
                for (int i = 0; i < size; ++i)
                    newData[i] = data[i];
                data = std::move(newData); // Eski diziyi yeni diziyle değiştir
            }
            data[size++] = value; // Yeni elemanı ekle
        }

        // doesContain fonksiyonu (belirli bir eleman kümede var mı kontrol ediyor)
        bool doesContain(int value) const {
            for (int i = 0; i < size; ++i) {
                if (data[i] == value) {
                    return true; // Eleman bulundu
                }
            }
            return false; // Eleman bulunamadı
        }
};