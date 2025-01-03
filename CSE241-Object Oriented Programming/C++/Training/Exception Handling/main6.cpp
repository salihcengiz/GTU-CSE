#include <iostream>
#include <stdexcept>
//except türev sınıfı (std::bad_alloc) kullandığımız için <stdexcept> include etmemiz gerekli


int main() {
    try {
        int* ptr = new int[100000000000]; // Büyük bir dizi bellek tahsisi başarısız olabilir
    }
    catch (const std::bad_alloc& e) { // new, allocation başarısız olursa std::bad_alloc fırlattığı için std::bad_alloc ile yakalıyoruz
        std::cout << "Hata: " << e.what() << std::endl;
        //throw bir şeyler fırlatmıyor da new kullanılıyorsa başarısız allocation durumunda what() fonksiyonu "std::bad_alloc" hatası döndürür 
        //Ayrıca özelleştirilmemiş bir istisna sınıfında kullanılırsa, genellikle "unknown exception" gibi varsayılan bir mesaj döndürür.

    }
    return 0;
}