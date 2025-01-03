#include <iostream>
#include "employee.h"
#include "hourlyemployee.h"
#include "salariedemployee.h"

using std::cout;
using std::endl;
using SavitchEmployees::Employee;
using SavitchEmployees::HourlyEmployee;
using SavitchEmployees::SalariedEmployee;


void showEmployeeData(const Employee object);

int main( ) {
    HourlyEmployee joe;
    joe.setName("Mighty Joe");
    joe.setSsn("123-45-6789");
    joe.setRate(20.50);
    joe.setHours(40);
    cout << "Check for " << joe.getName( )
        << " for " << joe.getHours( ) << " hours.\n";
    joe.printCheck( );
    cout << endl;

    SalariedEmployee boss("Mr. Big Shot", "987-65-4321", 10500.50);
    cout << "Check for " << boss.getName( ) << endl;
    boss.printCheck( );

    return 0;
}

/*
Sample Dialogue
Check for Mighty Joe for 40 hours.
________________________________________________
Pay to the order of Mighty Joe
The sum of 820 Dollars
________________________________________________
Check Stub: NOT NEGOTIABLE
Employee Number: 123-45-6789
Hourly Employee.
Hours worked: 40 Rate: 20.5 Pay: 820
_________________________________________________
Check for Mr. Big Shot
__________________________________________________
Pay to the order of Mr. Big Shot
The sum of 10500.5 Dollars
_________________________________________________
Check Stub NOT NEGOTIABLE
Employee Number: 987-65-4321
Salaried Employee. Regular Pay: 10500.5
_____________________________________________
*/