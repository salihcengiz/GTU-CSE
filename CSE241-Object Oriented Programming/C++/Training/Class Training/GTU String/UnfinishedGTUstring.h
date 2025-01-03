#include <iostream>
#include <cstring>
 
class GTUString {
public:
    // Constructors
    GTUString();                      // No-parameter constructor
    GTUString(const char* str);       // Constructor with char array
 
    // Destructor
    ~GTUString();
 
    // Member functions
    size_t len() const;               // Return the length of the string
    //GTUString substr(size_t start, size_t length) const; // Get a substring
 
    // Overloaded operators
    char& operator[](size_t index);          // Index operator for accessing characters
    const char& operator[](size_t index) const; // Const index operator
    GTUString& operator+=(const GTUString& rhs); // Append another GTUString
 
 
    // Friend functions
    friend bool operator==(const GTUString& lhs, const GTUString& rhs);
    friend bool operator!=(const GTUString& lhs, const GTUString& rhs);
    friend std::ostream& operator<<(std::ostream& os, const GTUString& str);
 
private:
    char* data;                       // Pointer to the character data
    size_t length;                    // Length of the string
};
 
// Global operator overloads for comparisons between GTUString objects
bool operator==(const GTUString& lhs, const GTUString& rhs);
bool operator!=(const GTUString& lhs, const GTUString& rhs);
 
std::ostream& operator<<(std::ostream& os, const GTUString& str) {
    if (str.data) {
        os << str.data;
    }
    return os;
}
 
// Default constructor
GTUString::GTUString() : data(new char[1]), length(0) {
    data[0] = '\0'; // Initialize to an empty string
}
 
// Constructor with char array
GTUString::GTUString(const char* str) {
    if (str) {
        length = std::strlen(str);
        data = new char[length + 1];
        std::strcpy(data, str);
    } else {
        data = new char[1];
        data[0] = '\0';
        length = 0;
    }
}
 
// Destructor
GTUString::~GTUString() {
    delete[] data;
}
 
// Return the length of the string
size_t GTUString::len() const {
    return length;
}
 
/* // Get a substring starting at 'start' with specified 'length'
GTUString GTUString::substr(size_t start, size_t length) const {
    if (start >= this->length || start + length > this->length) {
        return GTUString(); // Return an empty string if out of bounds
    }
 
    char* substr_data = new char[length + 1];
    std::strncpy(substr_data, data + start, length);
    substr_data[length] = '\0';
 
    GTUString result(substr_data);
    delete[] substr_data;
    return result;
} */
 
// Index operator for accessing characters
char& GTUString::operator[](size_t index) {
    if (index >= length) {
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}
 
// Const index operator
const char& GTUString::operator[](size_t index) const {
    if (index >= length) {
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}
 
// Global equality operator
bool operator==(const GTUString& lhs, const GTUString& rhs) {
    if (lhs.length != rhs.length) {
        return false;
    }
    return std::strcmp(lhs.data, rhs.data) == 0;
}
 
// Global inequality operator
bool operator!=(const GTUString& lhs, const GTUString& rhs) {
    return !(lhs == rhs);
}
 
 
GTUString& GTUString::operator+=(const GTUString& rhs) {
    char* new_data = new char[length + rhs.length + 1];
    std::strcpy(new_data, data);
    std::strcat(new_data, rhs.data);
 
    delete[] data; // Free old memory
    data = new_data;
    length += rhs.length;
    return *this;
}
 
 
int main() {
    GTUString str1("ABCD"), str2("C++!"), str3;
	{
		GTUString s5;
		s5 = str1;
		std::cout<< s5 << "\n";
	}
    if ("C++!" == str2)
        std::cout << "str2 is C++!\n";
	// prints B C D E
    for (int i = 0; i < str1.len(); ++i) {
		str1[i]++;
		std::cout << str1[i] << ' ';
    }
    // append str2 at the end of str1 
	str1 += str2; 
	//prints BCDEC++!
	std::cout << "\n" << str1 << "\n";
 
    return 0;
}