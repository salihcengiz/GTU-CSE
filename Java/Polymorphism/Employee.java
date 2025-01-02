public class Employee {
    private String name;
    private String ssn;
    private double netpay;

    public Employee() {
         setName("");
         setSsn("");
         setNetpay(0.0);
    }

    public String getName() {return name;}
    public String getSsn() {return ssn;}
    public double getNetpay() {return netpay;}

    public void setName(String newName) {name = newName;}
    public void setSsn(String newSsn) {ssn = newSsn;}
    public void setNetpay(double newNetpay) {netpay = newNetpay;}

    public void printCheck() {
        System.out.println("Name: " + getName() + ", SSN: " + getSsn() + ", Netpay: " + getNetpay());
    }
}