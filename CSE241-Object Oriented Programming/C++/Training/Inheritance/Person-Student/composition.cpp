#include <iostream>
#include <string>

//Instead of using inheritance, using composition between Student and Person

// Person class
class Person {
private:
    std::string name;
    int age;

public:
    Person(const std::string& name, int age) : name(name), age(age) {}

    std::string getName() const { return name; }
    int getAge() const { return age; }
};

// Student class containing a Person object
class Student {
private:
    Person person; // Composition: Student has a Person
    std::string studentID;

public:
    Student(const std::string& name, int age, const std::string& studentID)
        : person(name, age), studentID(studentID) {}

    std::string getName() const { return person.getName(); }
    int getAge() const { return person.getAge(); }
    std::string getStudentID() const { return studentID; }

    void display() const {
        std::cout << "Name: " << getName() << "\n"
                  << "Age: " << getAge() << "\n"
                  << "Student ID: " << getStudentID() << std::endl;
    }
};

int main() {
    Student student("Alice", 20, "S12345");
    student.display();
    return 0;
}
