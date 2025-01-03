#include <iostream>
#include <stdexcept>
//except türev sınıfı (std::invalid_argument) kullandığımız için <stdexcept> include etmemiz gerekli

void kontrol(int yas) {
    if (yas < 18) {
        throw std::invalid_argument("Yaş 18'den küçük olamaz!");
    }
}

int main() {
    try {
        kontrol(16);
    }
    catch (const std::invalid_argument& e) { //Catch'in aldığı argüman throw edilen class türünde olmalı (her zaman & ile gönder)
        std::cout << "Hata yakalandi: " << e.what() << std::endl; 
        //e.what class'ın aldığı string mesajını (bu durumda "Yaş 18'den küçük olamaz!") döndürür
    }
    return 0;
}
