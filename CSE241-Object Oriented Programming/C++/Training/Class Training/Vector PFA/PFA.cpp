#include "PFA.h"

// Constructor with optional capacity argument
PFA::PFA(int cap) : data() {
    data.reserve(cap); // Pre-allocate capacity
}

// Copy constructor
PFA::PFA(const PFA& o) : data(o.data) {}

// Destructor
PFA::~PFA() {
    // No need for manual cleanup since vector handles its own memory
}

// Assignment operator
PFA& PFA::operator=(const PFA& o) {
    if (this != &o) {
        data = o.data;
    }
    return *this;
}

// setCap: Resizes the vector's capacity
void PFA::setCap(int newCap) {
    if (newCap < data.size()) {
        data.resize(newCap); // Reduce size if new capacity is less than current size
    }
    data.reserve(newCap); // Reserve space without changing size if larger capacity is provided
}

// Adds an element if there's capacity left
void PFA::addElement(double d) {
    if (data.size() < data.capacity()) {
        data.push_back(d);
    } else {
        throw std::overflow_error("Capacity exceeded");
    }
}

// Removes the last element, reducing size
double PFA::removeElement() {
    if (isEmpty()) {
        throw std::underflow_error("No elements to remove");
    }
    double lastElement = data.back();
    data.pop_back();
    return lastElement;
}

// Checks if the array is empty
bool PFA::isEmpty() const {
    return data.empty();
}

// Empties the array by clearing the vector
void PFA::empty() {
    data.clear();
}

// Const subscript operator with range checking
double PFA::operator[](int i) const {
    if (i < 0 || i >= data.size()) {
        throw std::out_of_range("Index out of range");
    }
    return data[i];
}

// Non-const subscript operator with range checking
double& PFA::operator[](int i) {
    if (i < 0 || i >= data.size()) {
        throw std::out_of_range("Index out of range");
    }
    return data[i];
}
