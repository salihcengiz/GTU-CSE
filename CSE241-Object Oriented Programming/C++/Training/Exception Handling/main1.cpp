#include <iostream>
//except sınıfı kullanmadığımız için <stdexcept> include etmeye gerek yok

int main() {
    bool bir_sorun_var = false ; //Hata bayrağı (hata durumu yoksa false, hata durumu varsa true)

    try {
        // Hata olabilecek kod
        bir_sorun_var = true;
        
        if (bir_sorun_var) {
            throw "Bir hata oluştu!";
        }
    } catch (const char* hata) {
        // İstisna yakalandı
        std::cout << "Hata: " << hata << std::endl;
    }
    return 0;
}