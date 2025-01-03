#include <iostream>
#include <string>

// Class Templates: Bir sınıfın farklı veri türleriyle çalışmasını sağlamak için kullanılır

class Box1 { // Box1 sınıfı sadece int inputlar için çalışıyor
    public:
        Box1(int val) : value(val) {}
        void display() { std::cout << value << std::endl; }
    private:
        int value;
};

//template <class T> de aynı işlevi görür
template <typename T>
class Box2 { // Box2 artık her type için çalışıyor
    public:
        Box2(T val) : value(val) {}
        void display() { std::cout << value << std::endl; }
    private:
        T value;
};

int main() {
    Box1 intBox(5);
    intBox.display();  // Çıktı: 5

    Box2<double> doubleBox(42.5);
    doubleBox.display();  // Çıktı: 42.5

    Box2<std::string> strBox("Hello");
    strBox.display();  // Çıktı: Hello

    return 0;
}