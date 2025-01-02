class MyClass {
    int value;  // Heap'te saklanır
}

//Bir sınıfın her bir örneği (instance) kendine özgü değişkenler taşır. Bu değişkenlere örnek değişkenleri veya alanlar denir.
//Sınıfın kendisine değil, o sınıfın yaratılan nesnelerine ait olduklarından bu isim verilir.
class Person {
    String name;  // Instance variable (nesneye özel) - Heap'te saklanır
    int age;      // Instance variable (nesneye özel) - Heap'te saklanır
}

//Sınıf Düzeyinde Değişkenler (Statik Alanlar) Nedir?
//Statik değişkenler (static variables), bir sınıfa özgüdür ve o sınıfın tüm örnekleri (instances) tarafından paylaşılır.
//Bu değişkenler, sınıf yüklendiğinde bellekte yer alır ve program çalıştığı sürece varlığını sürdürür.
class Counter {
    static int count = 0; // Statik değişken (nesneye değil sınıfa özgü, yani sınıf düzeyinde değişken)
    //Ek bilgi: 'counter' Metaspace'de saklanır

    Counter() {
        count++; // Tüm nesneler için ortak (sınıf düzeyinde değişken)
    }
}

public class memoryManagement {
    public static void main(String[] args) {
        MyClass obj = new MyClass(); // obj referansı stack'te, ancak MyClass nesnesi ve 'value' heap'te ('obj' bir referans değişkendir)
        obj.value = 5;        

        Person p1 = new Person();
        p1.name = "Alice";
        p1.age = 25;

        Person p2 = new Person();
        p2.name = "Bob";
        p2.age = 30;
        // p1 ve p2 farklı nesnelerdir. p1 ve p2 için name ve age değişkenlerinin ayrı ayrı kopyaları heap'te saklanır.

        Counter c1 = new Counter();
        Counter c2 = new Counter();
        Counter c3 = new Counter();

        System.out.println(Counter.count); // 3 (statik değişken ortak)

    } // Bellek manuel olarak serbest bırakılmaz, Garbage Collector bunu halleder
}
