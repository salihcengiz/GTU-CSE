#ifndef PERSON_H
#define PERSON_H

#include <string>

using std::string;

namespace GTUSpace { // Person class'ı sadece GTUSpace namespace'inde geçerli, mainde bu namespace'i kullanmadan Person class'ına ulaşılamaz
    class Person {
        public:
            Person(const string& str) : name(str) {}
            void setName(const string& str) {name = str;}
            string getName() const {return name;}
        private: //Protected yapılırsa Person'dan kalıtılan sınıf da doğrudan bu üyelere erişebilir (private kaalırsa erişemez)
            string name;
    };
}

#endif