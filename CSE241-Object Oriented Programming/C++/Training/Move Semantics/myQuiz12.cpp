#ifndef SET_H
#define SET_H
#include <iostream>
#include <memory>
#include <utility>

using namespace std;

class Set {
    private:
        shared_ptr<int> p;
        int size;
        int cap;
    public:
        Set() : size(0), cap(50) {
            p = make_shared<int>(cap);
        }

        Set(const Set& o) : size(o.size), cap(o.cap) {
            p = make_shared<int>(cap);
            for (int i = 0; i < size; ++i)
                p[i] = o.p[i];
        }

        Set(Set&& o) noexcept: p(o.p), size(o.size), cap(o.cap) {
            o.p = nullptr;
            o.size = 0;
            o.cap = 0;
        }

        Set& operator=(Set&& o) noexcept {
            if (this == o)
                return *this;
            p = o.p;
            o.p = nullptr;
            o.size = 0;
            o.cap = 0;
        }
        
        ~Set(){
            p.reset();
        }

        Set& operator=(const Set& o) {
            if(this == o)
                return *this;

            p = make_shared<int>(o.cap);

            for (int i = 0; i < o.size; ++i) {
                p[i] = o.p[i];
            }
        }

        void addElement(int value) {
            p[size++] = value;
        }

        bool doesCo
}

#endif