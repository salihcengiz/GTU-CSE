#include "GtuString.h"

GTUString::GTUString(){
   MyDataCArray = new char[1];
   MyDataCArray[0] = '\0';
}
 
GTUString::GTUString(const char* cStr){
	int size = strlen(cStr);
	MyDataCArray = new char[size+1];
	strcpy(MyDataCArray, cStr);
}

GTUString::~GTUString(){
	delete [] MyDataCArray; //delete MyDataCArray; da yapabiliriz ama tüm array yerine sadece ilk karakteri deallocate eder
}

int GTUString::len() const{

	// return strlen(d); de tüm işlevi görmeye yeterdi

	int r = 0;
	char * p = MyDataCArray;
	while( *p != '\0'){
		++p;
		++r;
	}
	return r;

	/* Bu şekilde yaparsak da aynısı olur
	char * p = MyDataCArray;
	while( *p != '\0'){
		++p;
	}
	return p - MyDataCArray;
	*/
}
 
char & GTUString::at(int i){ //Vector fonksiyonu olan at gibi çalışıyor
	if (i < 0 || i >= len())
		exit(-1);
	return MyDataCArray[i];
}
 
 
char & GTUString::operator[](int i){
	if (i < 0 || i >= len()) //Tarama uzun olduğu için expensive, yapmamak senin elinde, compiler da yapmıyor
		exit(-1);
	return MyDataCArray[i];
}

char GTUString::operator[](int i) const{
	if (i < 0 || i >= len()) //Tarama uzun olduğu için expensive, yapmamak senin elinde, compiler da yapmıyor
		exit(-1);
	return MyDataCArray[i];
}

bool GTUString::operator==(const GTUString& o) const {
    if (len() != o.len()) return false;  // Different lengths, so can't be equal
    return strcmp(MyDataCArray, o.MyDataCArray) == 0;
}

bool GTUString::operator!=(const GTUString& o) const {
	return !(o == *this); 
}

void GTUString::Display() const {
	cout << MyDataCArray << endl; 
}