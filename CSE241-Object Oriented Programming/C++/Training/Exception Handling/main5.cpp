#include <iostream>
#include <stdexcept>
//except türev sınıfı (std::runtime_error) kullandığımız için <stdexcept> include etmemiz gerekli

void bolme(double a, double b) {
    if (b == 0) {
        throw std::runtime_error("Sifira bölme hatasi!");
    }
    std::cout << "Sonuç: " << a / b << std::endl;
}

int main() {
    try {
        bolme(10, 0);
    }
    catch (const std::exception& e) { //std::runtime_error, std::exception'dan türediği için onu std::exception ile yakalayabiliriz
        std::cout << "Hata: " << e.what() << std::endl;
        //e.what class'ın aldığı string mesajını (bu durumda "Sifira bölme hatasi!") döndürür
    }

    return 0;
}