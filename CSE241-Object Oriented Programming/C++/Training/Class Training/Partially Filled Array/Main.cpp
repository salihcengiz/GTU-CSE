#include "PFA.h"

int main() {
    int cap = 50;
    PFA stuff(cap);

    

    int number, enteredNumber = 0;
    int i = 0;

    cout<<"Please enter a number(for quit enter 241)\n";

    do{

    cin>>number;

    if (number != 241)
        stuff.addElement(number);
    

    }while(enteredNumber < stuff.getCapacity() && number != 241);


    for (int i = 0 ; i < stuff.getNumberUsed(); ++i)
        cout << stuff[i] << " ";
    cout<< endl;

    return 0;
}