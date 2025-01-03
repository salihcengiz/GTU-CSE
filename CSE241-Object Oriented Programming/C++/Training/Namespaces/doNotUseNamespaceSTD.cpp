#include <iostream>

using namespace std; //Bu dosyada using namespace std kullanmak mantıklı değil. Bunun 2 sebebi var.

class string { //STD namespace'te de bir tane olmasına rağmen kendi string class'ımı oluşturdum.
    public:
        void setString(char *s){str = s;}
    private:
        char *str;
};

int main() {   
    
    int cout = 7;
    //Burada cout adında bir değişken oluşturuyorum. Bu değişken ileride std::cout ile karışacak
        
    string myString;
    //Compiler burada benim string'imi mi yoksa std'ninkini mi kullancağına karar veremiyor. Kendi stringimi
    //kullanmak istiyorsam using namespace std; ifadesini kullanmamam lazım

    std::string myString; //Bu durumda ise bir karışıklık yok çünkü std'ye ait string'i kullandığımı belirtiyorum

    cout << cout; 
    //Burada soldaki cout'un stream insertion operator olarak çalışması gerekirken daha önce bir int cout oluşturduğum
    //için stream insertion operator da compiler tarafından int cout olarak algılanıyor

    std::cout << cout << std::endl; //Bu line terminale 7 basar
    //Burada ise karışıklık yok çünkü std'ye ait cout'u kullandığımı belirtiyorum
    
    return 0;
}

