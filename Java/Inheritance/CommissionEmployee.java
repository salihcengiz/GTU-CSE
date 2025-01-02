// Fig. 9.9: CommissionEmployee.java
// CommissionEmployee class represents a commission employee.

public class CommissionEmployee {
    protected String firstName;
    protected String lastName;
    protected String socialSecurityNumber;
    protected double grossSales; // gross weekly sales
    protected double commissionRate; // commission percentage

    // five-argument constructor
    public CommissionEmployee(String first, String last, String ssn, double sales, double rate) {
        // implicit call to Object constructor occurs here
        setFirstName(first);
        setLastName(last);
        setSocialSecurityNumber(ssn);
        setGrossSales(sales); // validate and store gross sales
        setCommissionRate(rate); // validate and store commission rate
    }

    public void setFirstName(String first) {firstName = first;}
    public String getFirstName() {return firstName;}

    public void setLastName(String last) {lastName = last;}
    public String getLastName() {return lastName;}

    public void setSocialSecurityNumber(String ssn) {socialSecurityNumber = ssn;}
    public String getSocialSecurityNumber() {return socialSecurityNumber;}

    public void setGrossSales(double sales) {grossSales = ( sales < 0.0 ) ? 0.0 : sales;}
    public double getGrossSales() {return grossSales;}

    public void setCommissionRate(double rate) {commissionRate = ( rate > 0.0 && rate < 1.0 ) ? rate : 0.0;}
    public double getCommissionRate() {return commissionRate;}

    public double earnings() {return commissionRate * grossSales;}

    public String toString() {
        return String.format("%s: %s %s\n%s: %s\n%s: %.2f\n%s: %.2f", "commission employee", firstName, lastName,
        "social security number", socialSecurityNumber, "gross sales", grossSales, "commission rate", commissionRate);
    }
}