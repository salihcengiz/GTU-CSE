//Here is the fixed code
 
#include<iostream>
#include<vector>
#include<list>
#include<forward_list>
#include<deque>
#include<exception>
 
using namespace std;
 
template <class T, class C> //template <class T, class C = vector<T>> şeklinde default parametre verilebilir
typename C::iterator findInContainer(C& c, const T& t) {
    for (auto it = c.begin(); it != c.end(); it++) {
        if (*it == t) {
            return it;
        }
    }
    throw runtime_error("element not found");
}
 
int main() {
    vector<int> vi = {1, 2, 3, 4, 5};
    vector<double> vd = {1.3, 2.3, 3.2, 4.2, 5.0};
    list<double> ld = {1.3, 2.3, 3.2, 4.2, 5.0};
    forward_list<double> fd = {1.3, 2.3, 3.2, 4.2, 5.0};
    deque<double> dd = {1.3, 2.3, 3.2, 4.2, 5.0};
 
    try {
        cout << "found element: " << *findInContainer(vi, 3) << endl;
        cout << "found element: " << *findInContainer(vd, 3.0) << endl;
        cout << "found element: " << *findInContainer(ld, 3.0) << endl;
        cout << "found element: " << *findInContainer(fd, 3.0) << endl;
        cout << "found element: " << *findInContainer(dd, 3.0) << endl;
    }
    catch (exception & e) {
        cout << "Caught exception: " << e.what() << endl;
    }
}

/* İlk Kod (Compile Olmayan)

#include<iostream>
#include<vector>
#include<list>
#include<forward_list>
#include<exception>
 
using namespace std;
 
 
// decltype (vector.begin())
template <class T, class C> 
class C::iterator findInContainer(const C<T> c, T t){ // findInContainer(const C& c, const T& t)
	for(auto it = c.begin(); it != c.end(); it++)
		if(*it == t)
			return it;
	throw runtime_error("element not found");
	// no return statement here
}
 
 
int main(){
	vector<int> vi = {1, 2, 3, 4, 5};
	vector<double> vd = {1.3, 2.3, 3.2, 4.2, 5.0};
	list<double> ld = {1.3, 2.3, 3.2, 4.2, 5.0};
    forward_list<double> fd = {1.3, 2.3, 3.2, 4.2, 5.0};
    dequeu<double> dd = {1.3, 2.3, 3.2, 4.2, 5.0};
 
 
	try{
	   cout << "found element" << *findInContainer(vi, 3);
	   cout << "found element" << *findInContainer(vd, 3.0);
	   cout << "found element" << *findInContainer(ld, 3.0);
	   cout << "found element" << *findInContainer(fd, 3.0);
	   cout << "found element" << *findInContainer(dd, 3.0);
	}
	catch(exception & e){
		cout << "Caught exception" << e.what();
	}
}
*/