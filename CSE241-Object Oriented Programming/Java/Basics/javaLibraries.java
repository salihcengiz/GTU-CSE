import java.util.Scanner;     // Scanner için
import java.util.ArrayList;   //
import java.util.Collections; //
import java.lang.String;      //String fonksiyonları için 
import java.lang.Math;        //Math fonksiyonları için 
import java.io.File;          //Dosya işlemleri için kullanılır.
import java.io.FileReader;    //Dosya işlemleri için kullanılır.
import java.io.FileWriter;    //Dosya işlemleri için kullanılır.

//Kütüphanelerin fonksiyonları ve açıklamaları kod örneğinden sonra yer alıyor

public class javaLibraries {
    public static void main(String[] args) {
        // Scanner nesnesi oluştur
        Scanner scanner = new Scanner(System.in); // Konsoldan veri almak için

        // Kullanıcıdan isim al
        System.out.print("Adinizi giriniz: ");
        String name = scanner.nextLine(); // Tam bir satırı alır.

        // Kullanıcıdan yaş al
        System.out.print("Yaşinizi giriniz: ");
        int age = scanner.nextInt(); // Tam sayı alır.

        // Kullanıcıdan favori sayı al
        System.out.print("Favori sayinizi giriniz: ");
        double favoriteNumber = scanner.nextDouble(); // Ondalık sayı alır.

        // Kullanıcıdan alınan bilgileri yazdır
        System.out.println("\n--- Bilgiler ---");
        System.out.println("Ad: " + name);
        System.out.println("Yaş: " + age);
        System.out.println("Favori Sayi: " + favoriteNumber);

        // Scanner'ı kapat
        scanner.close();
    }
}

/*
nextInt() ve nextLine() Kullanımı Arasındaki Sorun
Eğer nextInt() ve nextLine() peş peşe kullanılırsa, sorun çıkabilir. Örneğin:

    System.out.print("Bir sayı giriniz: ");
    int number = scanner.nextInt(); // Kullanıcı tam sayı girişi yapar
    System.out.print("Bir metin giriniz: ");
    String text = scanner.nextLine(); // Burada sorun oluşur

Bu durumda, nextLine() metodu, nextInt() tarafından bırakılan satır sonu karakterini (\n) okuyacağı için kullanıcıdan yeni bir girdi beklemeden
boş bir değer döndürür. Sorunu çözmek için, nextLine() çağrısından önce bir boş nextLine() eklenmelidir:

    System.out.print("Bir sayı giriniz: ");
    int number = scanner.nextInt();
    scanner.nextLine(); // Boş satır sonu karakterini temizler
    System.out.print("Bir metin giriniz: ");
    String text = scanner.nextLine();
*/

/*
Scanner:
Scanner(System.in): Konsoldan kullanıcı girişi alır.
nextInt(), nextDouble(), nextLine(): Kullanıcıdan farklı türlerde veri alır.

ArrayList:
add(E e): Listeye bir eleman ekler.
remove(int index): Belirtilen indisteki elemanı kaldırır.
get(int index): Belirtilen indisteki elemanı döner.
size(): Listedeki eleman sayısını döner.

Collections:
sort(List<T> list): Listeyi sıralar.
reverse(List<T> list): Listeyi ters çevirir.
shuffle(List<T> list): Listenin elemanlarını karıştırır.

String Fonksiyonları:
length(): String'in uzunluğunu döner.
charAt(int index): Belirtilen indisteki karakteri döner.
substring(int beginIndex, int endIndex): Belirtilen aralıktaki alt diziyi döner.
toLowerCase(), toUpperCase(): String'i küçük/büyük harfe çevirir.

Math Fonksiyonları:
abs(double a): Bir sayının mutlak değerini döner.
sqrt(double a): Bir sayının karekökünü döner.
pow(double a, double b): a üssü b değerini hesaplar.
random(): 0 ile 1 arasında rastgele bir sayı döner.

File:
exists(): Dosyanın var olup olmadığını kontrol eder.
createNewFile(): Yeni bir dosya oluşturur.
delete(): Dosyayı siler.
FileReader ve FileWriter:

FileReader(String fileName): Bir dosyayı okumak için kullanılır.
FileWriter(String fileName): Bir dosyaya yazmak için kullanılır.
*/