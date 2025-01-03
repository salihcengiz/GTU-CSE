#ifndef TEMPLATE_STACK
#define TEMPLATE_STACK

// Template class'ların member class function'larını daima header file'da implement et çünkü bazı compiler'lar ayrı bir implementation file ile
//compile etmeye izin vermiyor

// Burada yaptığımız şeye OODP method deniyor, araştır

#include <vector>
#include <list>
#include <deque>
#include <forward_list>

using std::vector;
using std::list;
using std::deque;
using std::forward_list;

template<class T, class Container = vector<T>> //Container için argüman girilmezse default parametre olarak vector atanır
class templateStack {
    public:
        templateStack(){}
        templateStack(const templateStack& o) {data = o.data;}
        void push(const T& value) {data.push_back(value);}
        void pop() {return data.pop_back();}
        bool empty() {return data.size() == 0;}
    private:
        R data;
};

#endif