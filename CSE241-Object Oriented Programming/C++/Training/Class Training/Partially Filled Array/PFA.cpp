#include "PFA.h"

PFA::PFA(int cap):numberUsed(0){
    capacity = (cap > 0) ? cap : 1;
    data = new double [capacity];
}

PFA::PFA(const PFA& other):numberUsed(other.getNumberUsed()), capacity(other.getCapacity()) {
    //delete [] data;   Hoca bunu yazmamış, sebebine bak, silinecek initial data olmaması muhtemel sebep
    data = new double [capacity];
    for (int i = 0; i < numberUsed; ++i)
        data[i] = other.data[i];
}  

PFA::~PFA(){
    assert(data != nullptr);
        delete [] data;
        data = nullptr; //Hocanın önemsediği bir detay
}

//Bunu daha verimli yap
//Bunu memcpy kullanarak daha da verimli yap
PFA& PFA::operator=(const PFA& other) {
    numberUsed = other.getNumberUsed();
    capacity = other.getCapacity();
    if(this != &other){ //& kullanımı önemli
        delete [] data;
        data = new double [capacity];
        for (int i = 0; i < numberUsed; ++i)
            data[i] = other.data[i];
    }
    else
        return *this;
}

double PFA::operator[](int i) const{
	if(i < 0 || i > getNumberUsed()) //Range check önemli
		exit(-1);
	return data[i];
}
 
double & PFA::operator[](int i){
	if(i < 0 || i > getNumberUsed()) //Range check önemli
		exit(-1);
	return data[i];
}

void PFA::addElement(double element){
    assert(numberUsed <= capacity);
    if(numberUsed == capacity){
    capacity *= 2;
    double *temp = new double [2*capacity];
    for (int i = 0; i < numberUsed; ++i)
        temp[i] = data[i];
    delete [] data;
    data = temp;
    }
    data[numberUsed++] = element;
}

double PFA::removeElement() {
  if (isEmpty()) {
    throw std::out_of_range("Cannot remove element from an empty PFA");
  }

  // Find the last non-empty element (optimized for potentially faster removal)
  int lastUsed = numberUsed - 1;
  while (lastUsed >= 0 && data[lastUsed] == 0.0) { // Check for 0.0 to avoid false positives
    lastUsed--;
  }

  // If no non-empty elements found, reset the PFA
  if (lastUsed < 0) {
    empty();
    return 0.0; // Or any default value if desired
  }

  // Shift elements to remove the last element
  double removedElement = data[lastUsed];
  for (int i = lastUsed; i < numberUsed - 1; ++i) {
    data[i] = data[i + 1];
  }

  // Decrement numberUsed and consider shrinking capacity
  numberUsed--;
  if (capacity > 4 * numberUsed) { // Adjust threshold as needed (e.g., 2 * numberUsed)
    capacity /= 2;
    double* temp = new double[capacity];
    std::copy(data, data + numberUsed, temp); // Use std::copy for efficiency
    delete[] data;
    data = temp;
  }

  return removedElement;
}