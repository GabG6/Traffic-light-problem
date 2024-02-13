#include "mbed.h"
#include <ostream>
#include <stdio.h>

DigitalOut mBuzzer;
DigitalOut mMotor;
DigitalOut mL1g;
DigitalOut mL1r;
DigitalOut mL2g;
DigitalOut mL2r;
DigitalOut mPedg;
DigitalOut mPedr;
DigitalIn mL2IRTraffic;
DigitalIn mL2IRWait;
DigitalIn mL1IRTraffic;
DigitalIn mL1IRWait;
DigitalIn mPedButton;
class TrafficLight;
class PedestrianLight;

/*class Interface{ // class that handles overriding and displaying the system
    private:
    Serial mMedium; Getstate getstate; Triggerevent Triggerevent;
    public:
    Interface(){}
    Interface(PinName port1, PinName port2):mMedium(port1,port2){}
    void display(){
        mMedium.printf("working");
    }
    char override(){
        if(mMedium.readable()){
            return mMedium.getc();
        }
    }
};*/

/**
 * @brief Class that computes a change in state of the sensor signals
 * @note Create separate objects for falling and rising edge
*/
class Flag{
    private:
    /**The previous signal*/
    bool prevState;
    /**Flag of the signal state change*/
    bool flag;
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
            flag = false;
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
            flag = true;
            prevState = sensor;
            return flag;
        } else{
            flag = false;
            prevState = sensor;
            return flag;
        }
    }

    /**
     * @brief Reset the flag
    */
    void resetFlag(){
        flag = false;
    }
};

class TrafficLight{
private:
    Timeout     safety;
    Timeout     waitTime;
    DigitalIn*  waitSensor;
    DigitalIn*  trafficSensor;
    DigitalOut* greenLight;
    DigitalOut* redLight;
    bool isGreenByDefault;
    bool controlFlag;
    int carCounter;
    bool priority1Flag;
    bool priority2Flag;

public:
    TrafficLight():isGreenByDefault(1), carCounter(0){}
    TrafficLight(bool gr_by_def, bool carcount):isGreenByDefault(gr_by_def), carCounter(carcount){}

    bool isInControl(){
        return controlFlag;
    }

    bool carArrived(){

    }

    bool carLeft(){

    }

    void update_state(TrafficLight *otherLight, PedLight *pedLight){

    }

    void greenLight(){

    }

    void redLight(){

    }
};

class PedLight{
private:
    Timeout safetyTimer;
    Timeout waitTimer;
    bool controlFlag;
    bool priority1Flag;
    bool priority2Flag;
public:

    void updateState(TrafficLight *tL1, TrafficLight *tL2){

    }

    void greenLight(){

    }
    void redLight(){

    }
};

int main() {
    while(1){

        wait(0.2);
    }
}