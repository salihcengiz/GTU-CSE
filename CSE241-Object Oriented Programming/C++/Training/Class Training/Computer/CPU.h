#ifndef CPU_H
#define CPU_H

class CPU {
    public:
        CPU();
        CPU(float clockSpeedV, int coresV);
        CPU(float clockSpeedV);
        CPU(int coresV);
        float getClockSpeed() const;
        void setClockSpeed(float clockSpeedV);
        int getCores() const;
        void setCores(int coresV);
    private:
        float clockSpeed;
        int cores;
        void testConstructorCPU(); //Constuctor'a invalid değerler gönderilmediğini kontrol etmek için daima test fonksiyonu kullan
};

#endif