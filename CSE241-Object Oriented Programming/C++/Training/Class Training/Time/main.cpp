#include <iostream>
#include "time.h"

using namespace std;

int main() {
    
    Time t1(14,45); //14:45
    Time t2(9);     //09:00
    Time t3;        //00:00
    Time t4(23,45); //23:45

    t1.printTime();
    t2.printTime();
    t3.printTime();
    t4.printTime();

    if(t1.isEqual(t2) || t1.isEarlier(t2))
        cout<<"Times are equal or t1 is earlier than t2\n";

    if(t1.isEqual(t4) || t1.isEarlier(t4))
        cout<<"Times are equal or t1 is earlier than t4\n";

    t1.addMinutes(80);
    t2.setHours(33);
    t2.setMinutes(t1.getMinutes());
    t4.addMinutes(30);

    t1.printTime();
    t2.printTime();
    t3.printTime();
    t4.printTime();

    return 0;
}