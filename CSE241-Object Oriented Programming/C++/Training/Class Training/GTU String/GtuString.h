#ifndef GTUST_H
#define GTUST_H
#include <iostream>
#include <cstring>
#include <cstdlib> 

using namespace std;

class GTUString {
   public:
	   GTUString();
	   GTUString(const char* cStr);
	   ~GTUString();
	   GTUString& operator+=(const GTUString& rhs); //Yazılmadı
	   //friend bool operator==(const GTUString& lhs, const GTUString& rhs); //Yazılmadı
       //friend bool operator!=(const GTUString& lhs, const GTUString& rhs); //Yazılmadı
	   char & operator[](int i); //Return type arrayi modify edebilmek için dereference, bu yüzden de const yok
	   char operator[](int i) const; //Return type char çünkü amaç sadece değeri okumak, bu yüzden de const var
	   int len() const;
       char & at(int i);
	   bool operator==(const GTUString& o) const;
	   bool operator!=(const GTUString& o) const;
	   void Display() const;
   private:
	   char* MyDataCArray; //Datayı tutmak için klasik c array
};

#endif