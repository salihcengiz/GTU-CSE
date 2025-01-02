/*Java'da "call by value" (değer ile çağrı) kavramı aslında vardır, ancak Java'daki referans tipi nesnelerin davranışları, genelde bu terimi 
kafa karıştırıcı hale getirir. Bu durumu açıklamak için adım adım ilerleyelim:

Call by Value Nedir?
Bir metodu çağırırken, argüman olarak verilen değerin bir kopyası metoda iletilir. Bu durumda, metodun içindeki değişiklikler, orijinal değişkeni 
etkilemez. Örneğin İlkel türlerle (primitive types) çalışırken bu barizdir (int x örneği)

Java ve Nesnelerde Referans Davranışı
Java'da tüm argümanlar call by value ile geçirilir. Ancak, referans tipi nesnelerle çalışırken, bu değerler referansın kendisinin kopyasıdır. 
Yani referansın kopyası, orijinal nesneyi işaret eder. şağıdaki MyClass obj örneğinde:

obj referans değişkenidir ve MyClass nesnesinin adresini taşır.
modifyObject metoduna bu referansın bir kopyası gönderilir.
Kopya, orijinal nesneyi işaret ettiği için, metot içinde yapılan değişiklikler nesne üzerinde etkili olur.

Neden "Call by Value" Var Ama Nesnelerde Call by Value Gibi Davranmıyor Gibi Görünüyor?
Java'da referansların kendisi birer değerdir. Metot çağrılırken referansın kopyası metoda iletilir. Bu yüzden:

Nesnenin Kendisi Değil, Referans Kopyalanır: Referans kopyalandığı için, metot içinde referansın işaret ettiği nesneyi değiştirebilirsiniz.
Ancak, referansı başka bir nesneye yönlendirmeye çalışırsanız, bu değişiklik dışarıdaki referansı etkilemez. (newObject örneği)
*/

class MyClass {
    int value;
}

public class CallByValue {
    public static void main(String[] args) {
        int x = 5;

        MyClass obj = new MyClass(); //obj referans değişkenidir ve MyClass nesnesinin adresini taşır.
        obj.value = 5;

        modifyValue(x); //Burada x değişkeninin değeri metoda kopyalanır. Metodun içinde yapılan değişiklikler, orijinal x değişkenini etkilemez.
        System.out.println(x); // Çıktı: 5 (orijinal değer değişmez)

        modifyObject(obj); //modifyObject metoduna bu referansın bir kopyası gönderilir.
        System.out.println(obj.value); // Çıktı: 10
        //Kopya, orijinal nesneyi işaret ettiği için, metot içinde yapılan değişiklikler nesne üzerinde etkili olur.

        MyClass newObject = new MyClass();
        newObject.value = 5;

        changeReference(newObject);
        System.out.println(newObject.value); // Çıktı: 5
    }

    public static void modifyValue(int num) {
        num = 10; // Sadece kopya değiştirilir
    }

    public static void modifyObject(MyClass o) {
        o.value = 10; // Orijinal nesneye erişim sağlanır
    }

    public static void changeReference(MyClass o) {
        o = new MyClass(); // Yeni bir nesneye yönlendirme (orijinal referansı etkilemez)
        o.value = 10;
    }
    //Bu durumda ne olur?
    //o referansının kopyası, metot içinde yeni bir nesneye yönlendirilir.
    //Ancak orijinal obj referansı, hala eski nesneyi işaret etmeye devam eder. Bu yüzden obj.value değişmez.
}
