#include <iostream>
//except sınıfı kullanmadığımız için <stdexcept> include etmeye gerek yok

int main() {
    try {
        throw 404; // Bir hata kodu atılıyor (int türünde atıldığı için catch argüman olarak int almalı)
    }
    catch (int hata_kodu) {
        std::cout << "Hata kodu yakalandi: " << hata_kodu << std::endl;
    }
    catch (...) { // Üstteki catch'lerin argüman türünde bir throw yakalanamadıysa diğer türlerdeki throwları yakalamak için bu syntax kullanılıyor
        std::cout << "Bilinmeyen bir hata yakalandi." << std::endl;
    }
    return 0;
}