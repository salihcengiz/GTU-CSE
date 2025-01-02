
void celcius_to_fah_kel (double celcius, double *fahrenheit, double *kelvin){

    
    *fahrenheit =(double)((9*celcius)/5) + 32;
    *kelvin = celcius +273.15;
    
}

void fahrenheit_to_cel_kel (double fahrenheit, double *celcius, double *kelvin){

    
    *celcius = (double)((fahrenheit - 32 )*5)/9;
    *kelvin = ((double)((fahrenheit - 32 )*5)/9)+ 273.15;

}

void kelvin_to_cel_fah (double kelvin,  double *celcius, double *fahrenheit){

    
    *celcius = kelvin - 273.15;
    *fahrenheit = (double)(9*(kelvin - 273.15))/5 + 32;
}

void print_temperatures (double celcius, double fahrenheit, double kelvin){

    printf("The degree in Celcius: %.2lf\n",celcius);
    printf("The degree in Fahrenheit: %.2lf\n",fahrenheit);
    printf("The degree in Kelvin: %.2lf\n",kelvin);
}