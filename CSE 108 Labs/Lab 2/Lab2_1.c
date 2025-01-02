#include <stdio.h>
#include <stdlib.h>

int LeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int AyinGunleri(int month, int year) {
    int ayingunleri[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && LeapYear(year))
        return 29;
    else
        return ayingunleri[month - 1];
}

int TarihlerArasiGun(int day1, int month1, int year1, int day2, int month2, int year2) {
    int totalDays = 0;

    for (int i = month1; i <= 12; ++i)
        totalDays += AyinGunleri(i, year1) - day1;

    for (int year = year1 + 1; year < year2; ++year) {
        totalDays += LeapYear(year) ? 366 : 365;
    }

    for (int i = 1; i < month2; ++i)
        totalDays += AyinGunleri(i, year2);

    totalDays += day2;

    return totalDays;
}

int main() {
    int day1, month1, year1, day2, month2, year2;

    printf("Ilk tarihi girin (gun ay yil): ");
    scanf("%d %d %d", &day1, &month1, &year1);

    printf("Ikinci tarihi girin (gun ay yil): ");
    scanf("%d %d %d", &day2, &month2, &year2);

    int totalDays = TarihlerArasiGun(day1, month1, year1, day2, month2, year2);
    printf("Iki tarih arasindaki gun sayisi: %d\n", totalDays);

    day1++;
    if (day1 > AyinGunleri(month1, year1)) {
        day1 = 1;
        month1++;
        if (month1 > 12) {
            month1 = 1;
            year1++;
        }
    }

    printf("Ilk tarihten sonraki gun: %d/%d/%d\n", day1, month1, year1);

    return 0;
}