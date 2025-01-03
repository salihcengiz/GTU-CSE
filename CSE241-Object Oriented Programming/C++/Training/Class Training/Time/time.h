#ifndef TIME_H
#define TIME_H

class Time {
    public:
        Time(int h = 0, int m = 0); //Constructor'u public'e koymayı unutma yoksa default olarak private olur.
        int getMinutes();
        int getHours();
        void printTime();
        void addMinutes(int m);
        bool isEarlier(Time &other);
        bool isEqual(Time &other);
        void setHours(int h);
        void setMinutes(int m);
    private:
        int hour;
        int minute;
};

#endif