// Fig. 9.11: BasePlusCommissionEmployeeTest.java
// Testing class BasePlusCommissionEmployee.

public class BasePlusCommissionEmployeeTest {
    public static void main(String args[]) {
        // instantiate BasePlusCommissionEmployee object
        BasePlusCommissionEmployee employee = new BasePlusCommissionEmployee("Bob", "Lewis", "333-33-3333", 5000, .04, 300 );

        // get base-salaried commission employee data
        System.out.println("Employee information obtained by get methods: \n");
        System.out.printf( "%s %s\n", "First name is", employee.getFirstName());
        System.out.printf( "%s %s\n", "Last name is", employee.getLastName());
        System.out.printf( "%s %s\n", "Social security number is", employee.getSocialSecurityNumber());
        System.out.printf( "%s %.2f\n", "Gross sales is", employee.getGrossSales());
        System.out.printf( "%s %.2f\n", "Commission rate is", employee.getCommissionRate());
        System.out.printf( "%s %.2f\n", "Base salary is", employee.getBaseSalary());

        employee.setBaseSalary(1000); // set base salary

        System.out.printf( "\n%s:\n\n%s\n","Updated employee information obtained by toString", employee.toString());
    }
}

/*
Employee information obtained by get methods:

First name is Bob
Last name is Lewis
Social security number is 333-33-3333
Gross sales is 5000.00
Commission rate is 0.04
Base salary is 300.00

Updated employee information obtained by toString:

base-salaried commission employee: Bob Lewis
social security number: 333-33-3333
gross sales: 5000.00
commission rate: 0.04
base salary: 1000.00 
*/