public class HourlyEmployee extends Employee {
    private double wageRate;
    private double hours;
 
    public HourlyEmployee() {
        super();
        setRate(0);;
        setHours(0);;
    }
 
    public HourlyEmployee(String theName, String theSsn, double thewageRate, double theHours) {
        super();
        setName(theName);
        setSsn(theSsn);
        setRate(thewageRate);
        setHours(theHours);
    }
 
    public double  getRate() {return wageRate;}
    public double  getHours() {return hours;}
  
    public void setRate(double newwageRate) {wageRate = newwageRate;}
    public void setHours(double newhoursworked) {hours = newhoursworked;}
  
    @Override
    public void printCheck() {
        System.out.println("Name:"+ getName()+"Ssn:" + getSsn() +"Netpay:" + getNetpay()+ "wageRate:"+ getRate()+"Hours :" + getHours());
    }
 }