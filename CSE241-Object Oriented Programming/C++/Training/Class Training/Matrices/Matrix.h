#ifndef MATRIX_H
#define MATRIX_H
#include <vector>   //Vector kullanımı için
#include <iostream> //ostream (>>) operator overloading için gerekli
#include <cstdlib>  //exit kullanımı için

using namespace std; //Vector kullanımı için

class Matrix2D {
    public:
        Matrix2D(int row = 1, int column = 1);
        int getNumOfRows() const;
        int getNumOfColumns() const;
        void setElement(int row, int column, int value);
        int getElement(int row, int column) const;
        Matrix2D operator+(const Matrix2D& other) const;
        bool operator==(const Matrix2D& other) const;
    private:
        //İki > sembolü arasında boşluk bırak, boşluksuzsa compiler eski sürümlerde right shift (>>) operatörüyle karıştırıyor
        vector<vector<int> > Matrix; //Matris için 2 boyutlu array
        bool isValid(int row, int column) const; // Girilen indexler vector için ulaşılabilir kontrol eden fonksiyon
        bool areSameSize(const Matrix2D& other) const; //2 matrisin boyutları aynı mı kontrol eden fonksiyon
};

//cout bana ait olmayan bir class (iostream'e ait), başkasının class'ı için kendi class'ım içinde bir fonksiyon yazmak doğru olmaz
//Bu yüzden başkasının class'larını kullanıyorsak bu fonksiyonları global tanımlamalıyız
ostream& operator<<(ostream& out, const Matrix2D matrix); 

//Normalde class member fonksiyon olarak yazmamız daha doğru ama bunu yapacak yöntemi şuan öğrenmedik
Matrix2D operator-(const Matrix2D& matrix); 

#endif

//Matrix = vector<vector<int> > (5, vector<int>(10, 0)) Tanımı:
//         [0,0,0,0,0,0,0,0,0,0]
//         [0,0,0,0,0,0,0,0,0,0]    5 => Row sayısı 
//Matrix = [0,0,0,0,0,0,0,0,0,0]    vector<int>(10, 0) => Her rowda bulunacak vektörlerin tanımı
//         [0,0,0,0,0,0,0,0,0,0]    vector<int>(10, 0) => 10 elemanlı ve her elemanı sıfır olan vektör
//         [0,0,0,0,0,0,0,0,0,0]    