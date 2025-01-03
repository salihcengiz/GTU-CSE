#include "GtuString.h"

int main() {

    GTUString str1, str2(""), str3("Salih");

    str1.Display();
    str2.Display();
    str3.Display();

    if (str1 == str2)
        cout << "str1 and str2 are equal" << endl ;
    else 
        cout << "str1 and str2 are not equal" << endl ;

    if (!(str1 != str3)) // if (str1 == str3) ile aynı şey
        cout << "str1 and str3 are equal" << endl ;
    else 
        cout << "str1 and str3 are not equal" << endl ;

    cout << str3[0] << endl;

    str3[0] = 'M';

    cout << str3.at(0) << endl;
    str3.Display();

    return 0;
}
 
