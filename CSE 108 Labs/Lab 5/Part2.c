#include <stdio.h>
#include "degree.h"

int main(){

    int choice;

    double user_celcius;
    double user_fahrenheit;
    double user_kelvin;

    double celcius;
    double fahrenheit;
    double kelvin;

    while(1){

    printf("Menu: \n");
    printf("0: Choose if you want to exit \n");
    printf("1: Convert to Fahrenheit (Enter Celcius degree) \n");
    printf("2: Convert to Celcius (Enter Fahrenheit degree) \n");
    printf("3: Convert to Kelvin (Enter Kelvin degree) \n");
    

    scanf("%d",&choice);

    switch(choice){
    
    case 0:

        printf("Exiting program.\n");
        return 0;

    case 1:

        printf("Please enter a celcius value: \n");
        scanf(" %lf", &user_celcius);

        celcius_to_fah_kel(user_celcius, &fahrenheit, &kelvin);

        print_temperatures(user_celcius, fahrenheit, kelvin);

        break;

    case 2:

        printf("Please enter a fahrenheit value: \n");
        scanf(" %lf", &user_fahrenheit);

        fahrenheit_to_cel_kel (user_fahrenheit, &celcius, &kelvin);

        print_temperatures(celcius, user_fahrenheit, kelvin);

        break;

    case 3:

        printf("Please enter a kelvin value: \n");
        scanf(" %lf", &user_kelvin);

        kelvin_to_cel_fah (user_kelvin, &celcius, &fahrenheit);

        print_temperatures(celcius, fahrenheit, user_kelvin);

        break;

    default:
        printf("Invalid input, choose again: \n");

        break;
        
    }

    }

    return 0;
}