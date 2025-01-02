// Fig. 9.10: BasePlusCommissionEmployee.java
// BasePlusCommissionEmployee inherits from CommissionEmployee and has access to CommissionEmployee's protected members.

public class BasePlusCommissionEmployee extends CommissionEmployee {
    private double baseSalary; // base salary per week

    // six-argument constructor
    public BasePlusCommissionEmployee(String first, String last, String ssn, double sales, double rate, double salary) {
        super(first, last, ssn, sales, rate);
        setBaseSalary( salary ); // validate and store base salary
    }

    // set base salary
    public void setBaseSalary( double salary ) {
        baseSalary = ( salary < 0.0 ) ? 0.0 : salary;
    }

    // return base salary
    public double getBaseSalary() {
        return baseSalary;
    }

    // calculate earnings
    public double earnings() {
        return baseSalary + ( commissionRate * grossSales ); // return baseSalary + super.earnings(); de yazılabilir
    }

    // return String representation of BasePlusCommissionEmployee       (yine burada da super.toString() kullanılabilir)
    public String toString() {
        return String.format("%s: %s %s\n%s: %s\n%s: %.2f\n%s: %.2f\n%s: %.2f","base-salaried commission employee",
        firstName, lastName, "social security number", socialSecurityNumber, "gross sales", grossSales, "commission rate", commissionRate,
        "base salary", baseSalary );
    }
}