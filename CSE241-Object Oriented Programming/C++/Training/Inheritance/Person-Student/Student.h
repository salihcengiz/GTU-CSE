#ifndef STUDENT_H
#define STUDENT_H

#include "Person.h"

using std::string;

//Burada prototyping yapılıyor
namespace { //verify sadece bu unnamed namespace'te tanımlı
    double verifyGPA(double d);
}

namespace GTUSpace { // Student class'ı sadece GTUSpace namespace'inde geçerli, mainde bu namespace'i kullanmadan Student class'ına ulaşılamaz
    class Student : public Person { 
    //Student inheritance ile person'ın tüm özelliklerini alıyor. Yani person'ın tüm private ve public data ve fonksiyonları student'ta da var.
    //Ancak Person'a ait private memberlara doğrudan ulaşılamıyor, public olan setter ve getterların kullanılması gerekli
    //Sadece public memberlarına ulaşabiliyor.(Person public tanımlandığı için)
    //Person'a ait private üyeler protected yapılırsa setter ve getterlar olmadan da private memberlara ulaşılabilir
        public:
            Student(const string& n, double gpa, const string& m) : Person(n), GPA(verifyGPA(gpa)), major(m) {}
            //Burada direkt name(n) yapmak mümkün değil, her zaman base class'a ait constructor kullanılmak zorunda
            //Person'a ait private üyeler protected yapılsa bile constructor kullanmak yerine name(n) yapılamaz
            void setGPA(double d) {GPA = verifyGPA(d);}
            double getGPA() const {return GPA;} 
            void setMajor(const string& m) { major = m;}
            string getMajor() const {return major;}
        private:
            double GPA;
            string major;
    };
}

// Checking for GPA errors with the help pf a global function placed in the unnamed namespace.
namespace { //verify sadece bu unnamed namespace'te tanımlı
    double verifyGPA (double d) {
        if (d >= 0.0 && d <= 4.0)
            return d;
        else
            return 0.0;
    }
}

#endif