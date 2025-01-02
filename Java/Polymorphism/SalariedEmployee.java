public class SalariedEmployee extends Employee{
    private double salary;

    public SalariedEmployee (){super();}
    public SalariedEmployee(String theName, String theSsn, double theWeeklySalary){
          super();
          setName(theName);
          setSsn(theSsn);
          setSalary(theWeeklySalary);
    }

    public void setSalary (double newSalary){salary = newSalary;}
    public double getSalary(){return salary;}

    @Override 
    public void printCheck(){
                System.out.println("Name: " + getName() + ", SSN: " + getSsn() + ", Salary :" + getSalary());

    }
}
