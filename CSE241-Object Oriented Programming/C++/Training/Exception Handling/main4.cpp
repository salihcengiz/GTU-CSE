#include <iostream>
#include <stdexcept>
//except türev sınıfı (std::runtime_error) kullandığımız için <stdexcept> include etmemiz gerekli

int main() {
    try {
        throw std::runtime_error("Runtime hatasi oluştu!");
    }
    catch (const std::exception& e) { //std::runtime_error, std::exception'dan türediği için onu std::exception ile yakalayabiliriz
        std::cout << "İstisna yakalandi: " << e.what() << std::endl;
        //e.what class'ın aldığı string mesajını (bu durumda "Runtime hatasi oluştu!") döndürür
    }
    return 0;
}