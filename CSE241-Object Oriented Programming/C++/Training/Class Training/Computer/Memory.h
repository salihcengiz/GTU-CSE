#ifndef MEMORY_H
#define MEMORY_H

class Memory {
    public:
        Memory();
        Memory(int totalCap, int usedMem);
        //Memory(int totalCap); //Diğer tek parametreli constructor ile overload olamadığı için kullanım dışı
        //Memory(int usedMem);  //Diğer tek parametreli constructor ile overload olamadığı için kullanım dışı
        int getTotalCapacity() const;
        void setTotalCapacity(int totalCap);
        int getUsedMemory() const;
        void setUsedMemory(int usedMem);   
    private:
        int totalCapacity;
        int usedMemory;
        void testConstructorMemory(); //Constuctor'a invalid değerler gönderilmediğini kontrol etmek için daima test fonksiyonu kullan
};

#endif