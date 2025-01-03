#include <iostream>
#include <vector>

using namespace std;

int main() {

    vector<int> vec; //Tek boyutlu proje
    vector<vector<int>> vec2d; //İki boyutlu array

    int next = 1;
    int choice = 0;

    while(next != 0) {
        cout << "Enter an element\n";
        cin >> next;
        if(next != 0)
            vec.push_back(next);
    }

    cout << "\nThe vector elements are: \n";

    for(int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }

    cout << "\nThe size of vector is " << vec.size()<< endl;

    cout << "\nThe capacity of vector is " << vec.capacity()<< endl;

    vec.reserve(vec.size() + 11);

    cout << "\nThe capacity of vector is " << vec.capacity()<< endl;

    cout << "\nWhich index do you want to know?" << endl;

    cin >> choice;

    while (choice >= vec.size()) {
        cout << "Enter a valid index number\n";
        cin >> choice;
    }

    cout << "The parameter is " << vec.at(choice) << endl;
    cout << "The parameter is " << vec[choice] << endl;

    vec.pop_back();

    for(int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }

    cout << endl;

    int first = vec.front();
    int last = vec.back();

    cout << "The first element is " << first << " and last element is " << last << endl;

    if(vec.empty())
        cout << "The vector is empty"<< endl;
    else
        cout << "The vector is not empty"<< endl;

    //Iteratorler

    vector<int>::iterator it;
    for ( auto it : vec)
        cout << it << " ";

    cout << endl;

    vec.erase(vec.begin()); //1. elemanı siler

    for(int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }

    cout << endl;

    vec.erase(vec.begin(), vec.begin() + 4); //Baştan ilk 4 elemanı siler

    for(int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }

    cout << endl;

    vec.insert(vec.begin(),5); // İlk sıraya 5 ekler

        for(int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }

    cout << endl;

    vec.insert(vec.begin() + 3 , 100); // Baştan 3. sıraya 100 ekler

        for(int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }

    cout << endl;
    
    return 0;
}