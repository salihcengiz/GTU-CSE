#include "PFA.h"
#include <iostream>

int main() {
    // Dynamically allocate a PFA object with an initial capacity of 20
    PFA* pfa = new PFA(20);

    // Add some elements to the PFA object
    try {
        pfa->addElement(1.1);
        pfa->addElement(2.2);
        pfa->addElement(3.3);

        // Display the elements
        for (int i = 0; i < pfa->getNumUsed(); ++i) {
            std::cout << "Element at index " << i << ": " << (*pfa)[i] << std::endl;
        }

        // Remove an element
        double removedElement = pfa->removeElement();
        std::cout << "Removed element: " << removedElement << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // Clean up by deleting the dynamically allocated PFA object
    delete pfa;

    return 0;
}
