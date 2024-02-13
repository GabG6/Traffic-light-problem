#include "mbed.h"
#include <ostream>
#include <stdio.h>

DigitalIn pin1(p19);
DigitalIn pin2(p16);
DigitalOut led1(LED1);
DigitalOut led2(LED2);

class Flag{
    private:
    /**The previous signal*/
    bool prevState;
    bool flag;
    /**Flag of the signal state change*/

    public:
    Flag(): prevState(false), flag(false){}

    /**
     * @brief Compute a change from low to high
     * @param sensor The current state of the sensor
     * @return The flag of the state change*/    
    bool carArrive(bool sensor){
        if(!prevState && sensor){
            flag = true;
            prevState = sensor;
            return flag;
        } else{
            //flag = false;
            prevState = sensor;
            return flag;
        }
    }

    /**
     * @brief Compute a change from high to low
     * @param sensor The current state of the sensor
     * @return The flag of the state change*/ 
    bool carLeft(bool sensor){
        if(prevState&& !sensor){
            prevState = sensor;
            flag = true;
            return flag;
        } else{
            //flag = false;
            prevState = sensor;
            return flag;
        }
    }
    void flagReset(){
        flag = false;
    }

};

Flag arrival;
Flag departure;
Serial pc(USBTX, USBRX);

int main() {
    while(1){
       if(arrival.carArrive(pin1)){
            pc.printf("car arrive \n\r");
            led1 = true;
        if(departure.carLeft(pin1)){
            pc.printf("car left\n\r");
            led2 = true;
            departure.flagReset();
            arrival.flagReset();
            wait(0.05);
            led2 = false;
        } else{
            pc.printf("car no leave\n\r");
            led2 = false;
        } 
        } else{
            pc.printf("car no arrive\n\r");
            led1 = false;
        } 
        /*if(sensorFlag.carLeft(pin1)){
            pc.printf("car left\n\r");
            led2 = true;
            sensorFlag.flagReset();
            wait(0.05);
            led2 = false;
        } else{
            pc.printf("car no leave\n\r");
            led2 = false;
        } */

    }
}