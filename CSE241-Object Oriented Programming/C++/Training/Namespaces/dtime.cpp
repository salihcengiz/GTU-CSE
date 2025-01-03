//This is the implementation file dtime.cpp of the class DigitalTime.
//The interface for the class DigitalTime is in the header file dtime.h.
#include <iostream>
#include <cctype>
#include <cstdlib>
using std::istream;
using std::ostream;
using std::cout;
using std::cin;
#include "dtime.h"

namespace { // Bu dosya için gerekli ama diğer dosyalar için gerekli olmayan fonksiyonlar unnamed namespace'te yazılır (encapsulation)
    int digitToInt(char c) {
        return ( int(c) - int('0') );
    }

    //Uses iostream, cctype, and cstdlib:
    void readMinute(int& theMinute) {
        char c1, c2;
        cin >> c1 >> c2;

        if (!(isdigit(c1) && isdigit(c2))) {
            cout << "Error: illegal input to readMinute\n";
            exit(1);
        }

        theMinute = digitToInt(c1)*10 + digitToInt(c2);

        if (theMinute < 0 || theMinute > 59) {
            cout << "Error: illegal input to readMinute\n";
            exit(1);
        }
    }

    //Uses iostream, cctype, and cstdlib:
    void readHour(int& theHour) {
        char c1, c2;
        cin >> c1 >> c2;

        if ( !( isdigit(c1) && (isdigit(c2) || c2 == ':' ) ) ) {
            cout << "Error: illegal input to readHour\n";
            exit(1);
        }

        if (isdigit(c1) && c2 == ':') {
            theHour = digitToInt(c1);
        }

        else { //(isdigit(c1) && isdigit(c2))
            theHour = digitToInt(c1)*10 + digitToInt(c2);
            cin >> c2; //discard ':'
            
            if (c2 != ':') {
                cout << "Error: illegal input to readHour\n";
                exit(1);
            }
        }

        if (theHour == 24)
            theHour = 0; //Standardize midnight as 0:00.

        if (theHour < 0 || theHour > 23) {
            cout << "Error: illegal input to readHour\n";
            exit(1);
        }
    }
} //unnamed namespace

namespace DTimeSavitch {
    //Uses iostream:
    istream& operator >>(istream& ins, DigitalTime& theObject) {
        readHour(theObject.hour);
        readMinute(theObject.minute);
        return ins;
    }
    ostream& operator <<(ostream& outs, const DigitalTime& theObject) {}
    //<The body of the function definition is the same as in Display 11.2.>

    bool operator ==(const DigitalTime& time1, const DigitalTime& time2) {}
    //<The body of the function definition is the same as in Display 11.2.>

    DigitalTime::DigitalTime(int theHour, int theMinute) {}
    //<The body of the function definition is the same as in Display 11.2.>

    DigitalTime::DigitalTime( ) {}
    //<The body of the function definition is the same as in Display 11.2.>

    int DigitalTime::getHour( ) const {}
    //<The body of the function definition is the same as in Display 11.2.>

    int DigitalTime::getMinute( ) const {}
    //<The body of the function definition is the same as in Display 11.2.>

    void DigitalTime::advance(int minutesAdded) {}
    //<The body of the function definition is the same as in Display 11.2.>

    void DigitalTime::advance(int hoursAdded, int minutesAdded) {}
    //<The body of the function definition is the same as in Display 11.2.>

} //DTimeSavitch