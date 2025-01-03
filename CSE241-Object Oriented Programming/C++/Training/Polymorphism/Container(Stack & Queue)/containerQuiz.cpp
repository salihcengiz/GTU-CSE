#include <vector>
#include <list> //Bunu unuttun, list kullanacaksan aynı vector gibi <list>'i de eklemelisin

using std::vector;
using std::list;

class Container {
    public:
        // default constructor eklenebilir
        virtual void add(int value) = 0; 
        virtual int remove() = 0;
        virtual bool empty() const = 0; // Sen bu fonksiyonu virtual yazmadın (direkt "bool empty() const;" şeklinde yazdın)
    protected: // Sen burada private yaptın, türetilen sınıfların add ve remove fonksiyonlarında dataya ulaşabilmen için protected yapman lazım
        list<int> data;
};

// LIFO
class Stack: public Container{
    public:
        Stack(): Container(){} //Sen constructor yazmayı unuttun
        void add(int value) {data.push_back(value);}
        int remove() {
			int backValue = data.back(); // Son elemanı al
			data.pop_back(); // Son elemanı kaldır
			return backValue; // Alınan değeri döndür
		}
        bool empty() const {return data.size() == 0;} //Bunu hiç yazmadın
};

// FIFO
class Queue : public Container {
    public:
        Queue(): Container(){} //Sen constructor yazmayı unuttun
        void add(int value) { data.push_back(value);}
        int remove() {
			int frontValue = data.front(); // İlk elemanı al
			data.pop_front(); // İlk elemanı kaldır
			return frontValue; // Alınan değeri döndür
		}
        bool empty() const { return data.size() == 0;} //Bunu hiç yazmadın
};

// Pointer olarak yazmak zorundasın çünkü abstract bir sınıfın bir objesi oluşturulamaz, sadece bu abstract sınıftan türeyecek olan diğer
// sınıfların (örneğin burada Stack ve Queue) objelerinin referansıni işaret etmek için pointer'ı tutulabilir
void removeEverything(vector<Container*> &v){ //Neden & koyman gerekti bak
	for(int i = 0; i < v.size(); ++i){
		Container* vec = v[i];  //Yine aynı sebepten bir Container objesi oluşturamazsın, sadece bir Container pointer'ı oluşturabilirsin
		while(!vec->empty())
			vec->remove();
	}
}

/* Bu da senin hatalı fonksiyonun
    void removeContainer(vector<Container*> vec){ 
        for(int i = 0; i < vec.size(); ++i) {
            for(int j = 0; j < vec[i]->data.size(); ++j) {
                vec[i]->data.remove();
            }
        }
    }
*/

int main(){

    // Container c; Burada yorum satırını kaldırırsan böyle bir obje oluşturamayacağını görürsün

    Queue a1, a2;
	Stack b1, b2;

	a1.add(10); // a1 = {10}
    a1.add(20); // a1 = {10,20}

	b1.add(30); // b1 = {30}
    b1.add(40); // b1 = {30, 40}

	a2.add(50); // a2 = {50}
    a2.add(60); // a2 = {50,60}

	b2.add(70); // b2 = {70}
    b2.add(80); // b2 = {70, 80}
	
    vector<Container*> myVector; // Yine aynı sebepten bir Container vector oluşturamazsın, sadece bir Container pointer vector oluşturabilirsin
    // Sen burada vector<Container> myVector yaptın yani pointer koymayarak hata yaptın
    
	myVector.push_back(&a1); //Sen burada hep & kullanmadan push'ladın. Neden doğru olanın bu olduğunu araştır
	myVector.push_back(&a2); //Sen burada hep & kullanmadan push'ladın. Neden doğru olanın bu olduğunu araştır
	myVector.push_back(&b1); //Sen burada hep & kullanmadan push'ladın. Neden doğru olanın bu olduğunu araştır
	myVector.push_back(&b2); //Sen burada hep & kullanmadan push'ladın. Neden doğru olanın bu olduğunu araştır
 
    // Ayrıca push_back() yerine vector<Container> myVector = {a1, a2, b1, b2}; yapmayı denedin. Bu yöntem çalışıyor mu dene

	removeEverything(myVector);

    return 0;
}

//Container is an abstract class with no function implementations. Stack is a First-In-First-Out class, Queue is a Last-In-First-Out class.
//STL list class has functions push_back, pop_back, push_front, and pop_front. Implement all classes in a single file. In the same file,
//write a global function that that takes a vector<Container*>. The function removes all the elements in the Containers. Write a main 
//function that calls this global function with Stack and Queue objects.