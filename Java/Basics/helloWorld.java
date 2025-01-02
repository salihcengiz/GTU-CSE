public class helloWorld { 
    // Java'da her şey (C++'taki tüm dosyaya karşılık gelen her şey) bir sınıfın içinde tanımlanır
    // Bu sınıf en dıştaki public sınıftır. Adı her zaman dosya adı ile aynı olmalıdır
    

    public int toplama(int a, int b) { // Fonksiyon sistemi aynı
        return a + b;
    }

    // Neden Run | Debug yazıyor araştır, gerekliyse kaldır
    public static void main(String[] args) {   //Main fonksiyon: public static void main(String[] args): Programın giriş noktasıdır.
        System.out.println("Hello, World!"); //System.out.println("Hello, World!");: Konsola çıktı yazdırır.
        // Burada çıkan x: ifadesini araştır

        //Değişken Tanımlama: Java'da veri türlerini belirtmek zorunludur
        int number = 10; // Tamsayı
        String text = "Merhaba"; // Metin
        double pi = 3.14; // Ondalıklı sayı
        boolean isJavaFun = true; // Mantıksal değer

        if (number > 5) { // Koşullu ifade sistemi aynı
            System.out.println("Sayi 5'ten büyük");
        }
        
        for (int i = 0; i < 5; i++) { // Döngü sistemi aynı
            System.out.println(i);
        }

        //Java'da da tıpkı C++'ta olduğu gibi fonksiyonlar (Java'da bunlara "metot" denir) main dışında tanımlanabilir.
        //Ancak Java'da tüm metotlar bir sınıfın içinde yer alır. Java'nın nesne yönelimli (OOP) yapısı gereği, her şey sınıf ve nesneye bağlıdır.
        helloWorld calc = new helloWorld(); // Sınıfın bir nesnesi
        int sonuc = calc.toplama(5, 10);    // Metot çağrısı
        System.out.println("Sonuç: " + sonuc);

        if (isJavaFun) {
            System.out.println(number);
            System.out.println(text);
            System.out.println(pi);
        }
    }
}

//Derleme (Compile): javac helloWorld.java (Bu, bir helloWorld.class dosyası oluşturur)
//Çalıştırma (Run): java helloWorld

/*
public static void main(String[] args) ve System.out.println("Hello, World!"); ne ifade ediyor?
public static void main(String[] args):

public: Bu fonksiyonun her yerden erişilebilir olduğunu belirtir.
static: Sınıfın bir örneği (instance) olmadan çalıştırılabileceğini ifade eder. Program başlatılırken JVM (Java Virtual Machine) bu fonksiyonu 
doğrudan çağırır. Ayrıca class'ın sadece bu dosya için geçerli olduğunu ifade eder
void: Bu fonksiyonun bir değer döndürmeyeceğini belirtir.
String[] args: Komut satırından programa argümanlar gönderildiğinde bu parametreyle yakalanır. Örneğin, java helloWorld arg1 arg2 çalıştırıldığında
 args[0] = "arg1", args[1] = "arg2" olur.
System.out.println("Hello, World!");:

System: Java'nın önceden tanımlı bir sınıfıdır.
out: System sınıfının bir özelliği olan PrintStream nesnesidir. Konsola yazdırma işini yapar.
println: Konsola bir satır yazdırır ve ardından alt satıra geçer. "Hello, World!" metnini ekrana yazdırır.
 */
