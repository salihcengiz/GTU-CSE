#include <string>
#include <vector>

using namespace std;

//Class for a pair of values of type T:
template<class T>
class Pair {
    public:
        Pair();
        Pair(T firstValue, T secondValue);
        void setFirst(T newValue);
        void setSecond(T newValue);
        T getFirst() const;
        T getSecond() const;
    private:
        T first;
        T second;
};

template<class T>
Pair<T>::Pair(T firstValue, T secondValue) :first(firstValue), second(secondValue) {}

template<class T>
void Pair<T>::setFirst(T newValue) {
    first = newValue;
}
template<class T>
T Pair<T>::getFirst() const {
    return first;
}

int main() {
    Pair<char> cp('A', 'B');
    Pair<char> ip(5, 10);
    Pair<char> dp(2.5, 7.3);
    Pair<string> sp("Hello", "World");

    vector<int> a, b;
    Pair<vector<int>> v(a,b);
    Pair<Pair<char>> p(cp,cp);

    return 0;
}