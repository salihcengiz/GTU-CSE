#include <iostream>
#include "time.h"

using namespace std;

void Time::setHours(int h){
    if( h >= 0 && h < 24)
        hour = h;
    else
        hour = 0;
}

void Time::setMinutes(int m){
    if( m >= 0 && m < 60)
        minute = m;
    else
        minute = 0; 
}

Time::Time(int h, int m){
    setHours(h);
    setMinutes(m);
}

int Time::getMinutes(){
    return minute;
}

int Time::getHours(){
    return hour;
}

void Time::printTime(){
    if(getHours() < 10)
        cout<<"0";
    cout<<getHours();
    cout<<":";
    if(getMinutes() < 10)
        cout<<"0";
    cout<<getMinutes()<<endl;
}

bool Time::isEarlier(Time &other){
    if((getHours() < other.getHours()) || (getHours() == other.getHours() && getMinutes() < other.getMinutes()))
        return true;
    else
        return false;
}

bool Time::isEqual(Time &other){
    if((getHours() == other.getHours() && getMinutes() == other.getMinutes()))
        return true;
    else
        return false;
}

void Time::addMinutes(int m){
    if(minute < 0)
        cerr<<"Minute can not be negative";
    else{
        setHours((getHours() + (getMinutes() + m) / 60) % 24);
        setMinutes((getMinutes() + m) % 60);
    }
}       
