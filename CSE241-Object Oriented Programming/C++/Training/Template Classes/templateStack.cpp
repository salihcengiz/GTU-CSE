#include "templateStack.h"

int main() {
    templateStack<int> s1; //Container için argüman girilmediğinden dolayı container için default parametre olarak vector atandı
    templateStack<int, list<int>> s2;
    templateStack<char, list<int>> s3; //Neden error vermiyor? Sonuçta int list'e char koyamam.
    //templateStack<int, Money> moneyStack; böyle bir şey neden olamaz?
    //templateStack<int, array<int>> arrayStack; (burada array std::array) böyle bir şey neden olamaz?
    return 0;
}