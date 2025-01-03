#include <memory>
#include "PFA.h"
#include <iostream>

int main() {
    // Use a smart pointer to manage the PFA object
    std::unique_ptr<PFA> pfa = std::make_unique<PFA>(20);

    // Same usage as before
    pfa->addElement(1.1);
    pfa->addElement(2.2);
    pfa->addElement(3.3);

    for (int i = 0; i < pfa->getNumUsed(); ++i) {
        std::cout << "Element at index " << i << ": " << (*pfa)[i] << std::endl;
    }

    // No need to delete; std::unique_ptr automatically deallocates memory
    return 0;
}
