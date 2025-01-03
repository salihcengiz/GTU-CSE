#include "Rational.h"

using namespace std;

void sort(vector<Rational> &vec); //Bunu yazmayı unuttun

vector<Rational> findMedians(vector<vector<Rational>> &vec){ //Sen &vec yerine vec yazdın, &vec daha verimli
    vector<Rational> result;
    for(int i = 0 ; i < vec.size() ; ++i){ 
        sort(vec[i]);
        int size = vec[i].size();
        if(size % 2 == 1)
            result[i] = vec[i][size/2];     //Sen result[i] = vec[i][size/2 + 1]; yazdın
            
        else
            result[i] = ((vec[i][size/2 - 1] + vec[i][size/2]) / 2); //Sen result[i] = ((vec[i][size/2 - 1] + vec[i][size/2]) / 2);
    }
    return result;
}

void sort(vector<Rational> &vec){
    for(int i = 0 ; i < vec.size() - 1; ++i){              //Sen for(int i = 0 ; i < vec.size() - 1 ; ++i) yazdın
        for(int j = 0 ; j < vec.size() - i - 1; ++j){   //Sen for(int j = 0 ; j < vec.size(); ++j)
            if(vec[j] > vec[j + 1]){                        //Sen if(vec[i] > vec[j]){  
                Rational temp = vec[j];                 //Sen Rational temp = vec[i];   
                vec[j] = vec[j + 1];                        //Sen vec[j] = vec[j];   
                vec[j + 1] = temp;                          //Sen vec[j] = temp; 
            }
        }
    }
}

int main() {

    Rational r1(3,5), r2(2,7), r3(1,2), r4(7,8), r5(1,5), r6(5,7), r7(1,3);

    vector<Rational> vec1 = {r1, r2, r3, r4};   // 2/7, 1/2, 3/5, 7/8   => 11/10 
    vector<Rational> vec2 = {r5, r6, r7};       // 1/5, 1/3, 5/7        => 1/3
    vector<Rational> vec3 = {r1, r3, r7};       // 1/3, 1/2, 3/5        => 1/2
    vector<Rational> vec4 = {r2, r4, r6, r7};   // 2/7, 1/3, 5/7, 7/8   => 22/21
    vector<Rational> vec5 = {r3, r4, r5};       // 1/5, 1/2, 7/8        => 1/2

    vector<vector<Rational>> vec2D = {vec1, vec2, vec3, vec4, vec5};

    vector<Rational> result = findMedians(vec2D);

    for (int i = 0; i < result.size(); ++i) {
        Rational temp = result[i];
        temp.print();
    }
    
    return 0;
}