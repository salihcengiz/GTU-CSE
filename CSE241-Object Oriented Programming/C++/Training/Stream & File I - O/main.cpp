#include <fstream> // File operations için gerekli kütüphane
#include <iostream>
#include <string>

void copyStream(std::istream &input, std::ostream &output) {
    char buffer[101]; // 100 karakter + null terminator
    input.read(buffer, 100); // 100 karakter oku
    buffer[input.gcount()] = '\0'; // Son okunan karakterden sonra null terminator ekle
    output.write(buffer, input.gcount()); // Okunan karakter sayısı kadar yaz
}

int main() {
    // 1. Standart giriş ve çıkış ile kullanım (cin ve cout)
    std::cout << "Enter some text (at least 100 characters): ";
    copyStream(std::cin, std::cout);
    std::cout << std::endl;

    // 2. Dosyalarla kullanım
    std::ifstream inputFile("input.txt");
    std::ofstream outputFile("output.txt");

    if (!inputFile || !outputFile) {
        std::cerr << "Error opening files!" << std::endl;
        return 1;
    }

    copyStream(inputFile, outputFile);

    std::cout << "100 characters copied from input.txt to output.txt." << std::endl;

    inputFile.close();
    outputFile.close();

    return 0;
}
