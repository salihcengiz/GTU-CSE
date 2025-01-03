//This is the header file pfarrayd.h. This is the interface for the
//class PFArrayD. Objects of this type are partially filled arrays of doubles.
//It would be good to place this class in a namespace, but we have not done so in order to keep the example simple.

#ifndef PFARRAYD_H
#define PFARRAYD_H

class PFArrayD {
    public:
        PFArrayD( ); //Initializes with a capacity of 50.
        PFArrayD(int capacityValue);
        PFArrayD(const PFArrayD& pfaObject);
        ~PFArrayD( );
        void addElement(double element); //Precondition: The array is not full. Postcondition: The element has been added.
        bool full( ) const; //Returns true if the array is full, false otherwise.
        int getCapacity( ) const;
        int getNumberUsed( ) const;
        void emptyArray( ); //Resets the number used to zero, effectively emptying the array.
        double& operator[](int index); //Read and change access to elements 0 through numberUsed - 1.
        PFArrayD& operator =(const PFArrayD& rightSide);
    protected:  //Dikkat: Burada private yerine protected yapmak kalıtılan sınıfın private dataya doğrudan ulaşması için önemli
                //Eğer data protected yerine private olursa kalıtılan sınıf bu datalara doğrudan ulaşamaz (public fonksiyonlar ile ulaşabilir)
        double *a; //for an array of doubles.
        int capacity; //for the size of the array.
        int used; //for the number of array positions currently in use.
};
#endif //PFARRAYD_H