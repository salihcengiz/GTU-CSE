// Fig. 3.11: GradeBookTest.java
// GradeBook constructor used to specify the course name at the
// time each GradeBook object is created.

public class GradeBookTest {
    // main method begins program execution
    public static void main(String args[]) {
        // create GradeBook object
        GradeBook gradeBook1 = new GradeBook("CS101 Introduction to Java Programming");
        GradeBook gradeBook2 = new GradeBook("CS102 Data Structures in Java");

        // display initial value of courseName for each GradeBook
        System.out.printf( "gradeBook1 course name is: %s\n", gradeBook1.getCourseName() );
        System.out.printf( "gradeBook2 course name is: %s\n", gradeBook2.getCourseName() );
    }
}
    
