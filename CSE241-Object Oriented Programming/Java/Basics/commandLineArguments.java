/*
Java'da public static void main(String[] args) metodu, programın başlangıç noktasıdır. Buradaki String[] args, komut satırından program 
çalıştırılırken geçirilen komut satırı argümanlarını temsil eder. Bu argümanlar, program başlatıldığında dışarıdan girilen değerlerdir 
ve bir dizi (array) olarak programa aktarılır.

Komut Satırı Argümanlarının Kullanımı
Tanım: String[] args, program çalıştırılırken verilen tüm argümanları bir dizi içinde tutar.

Dizi boyutu, argüman sayısına eşittir.
Her argüman, bir String olarak saklanır.
Kullanım: Program çalıştırılırken komut satırında argümanlar aşağıdaki gibi belirtilir:

java ProgramAdı arg1 arg2 arg3

rg1, args[0]'a atanır.
arg2, args[1]'e atanır.
arg3, args[2]'ye atanır.



*/

public class commandLineArguments {
    public static void main(String[] args) {
        // Argüman sayısını yazdır
        System.out.println("Argüman Sayisi: " + args.length);

        // Her bir argümanı yazdır
        for (int i = 0; i < args.length; i++) {
            System.out.println("Argüman " + i + ": " + args[i]);
        }

        //Veya

        if (args.length < 2) {
            System.out.println("Lütfen iki sayi giriniz.");
            return;
        }

        int num1 = Integer.parseInt(args[0]);
        int num2 = Integer.parseInt(args[1]);

        System.out.println("Toplam: " + (num1 + num2));
    }
}
