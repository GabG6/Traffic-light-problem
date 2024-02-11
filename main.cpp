#include "mbed.h"
#include <stdio.h>


class Interface{ // class that handles overriding and displaying the system
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
};

/**
 * @brief Returns a flag only if the traffic light sensor goes from 1 to 0
*/
class Flag {
private:
    bool state;
    bool flag;

public:
    Flag() : state(false), flag(false) {}
    /**
     * @brief Activate flag only when the sensor signal goes from 1 to 0
     * @param newstate the current state of the input
     * @var state - the previous state of the input
    */
    void setState(bool newState) {
        if (state && !newState) {
            flag = true;
        } else {
            flag = false;
        }
        state = newState;
    }

    /**
     *@return flag*/
    bool isFlagSet() const {
        return flag;
    }
    /**
     * @brief reset flag
    */
    void resetFlag() {
        flag = false;
    }
};

class Getstate{
    private:
    DigitalIn mL2IRclose;
    DigitalIn mL2IRfar;
    DigitalIn mL1IRclose;
    DigitalIn mL1IRfar;
    DigitalIn mPedButton;
    Flag carFlag;
    
    public:
    Getstate():
        mL2IRclose(p20),
        mL2IRfar(p19),
        mL1IRclose(p18),
        mL1IRfar(p17),
        mPedButton(p16){}

    
    /**
     * @brief check if a car has passed L1
     * @return boolean 1 or 0
    */
    bool hasL1Passed(){
        for(int i = 0; i<10; i++){ /**perform multiple checks*/
            carFlag.setState(mL1IRclose);
            if(carFlag.isFlagSet()){
                return true;
            }
        }
        return false;
        }
    /**
     * @brief check if a car has passed L2
     * @return boolean 1 or 0
    */
    bool hasL2Passed(){
        for(int i = 0; i<10; i++){
            carFlag.setState(mL2IRclose);
            if(carFlag.isFlagSet()){
                return true;
            }
        }
        return false;
    }
    /**
     * @brief check if a car is waiting L1
     * @return boolean 1 or 0
    */
    bool isL1Waiting(){
        int averagecount = 0;
        for(int i=0; i<10; i++){
            if(mL1IRfar){
                averagecount++;
            }
        }
        return (averagecount>=7);
        }
    /**
     * @brief check if a car is waiting L2
     * @return boolean 1 or 0
    */
    bool isL2Waiting(){
        int averagecount = 0;
        for(int i=0; i<10; i++){
            if(mL2IRfar){
                averagecount++;
            }
        }
        return (averagecount>=7);
        }
    /**
     * @brief check if a pedestrian is waiting
     * @return boolean 1 or 0
    */
    bool isPedWaiting(){
    for(int i=0; i<10; i++){
        if(mPedButton){
            return 1;
        }
        else{
            return 0;
        }
    }
    }
};

class Triggerevent{

    private:
    DigitalOut mBuzzer;
    DigitalOut mMotor;
    DigitalOut mL1g;
    DigitalOut mL1r;
    DigitalOut mL2g;
    DigitalOut mL2r;
    DigitalOut mPedg;
    DigitalOut mPedr;
    
    /**
     * @brief actuator container
     * @param l1g    lane 1 green led
     * @param l2g    lane 2 green led
     * @param pedg   pedestrian green led
     * @param l1r    lane 1 red led
     * @param l2r    lane 2 red led
     * @param pedr   pedestrian red led
     * @param buzzer pedestrian buzzer
     * @param motor  pedestrian haptic motor
    */
    void setLights(bool l1g, bool l2g, bool pedg, bool l1r, bool l2r, bool pedr, bool buzzer, bool motor) {
    mL1g = l1g;
    mL2g = l2g;
    mPedg = pedg;
    mL1r = l1r;
    mL2r = l2r;
    mPedr = pedr;
    mBuzzer = buzzer;
    mMotor = motor;
}

    public:
    Triggerevent():
        mMotor(p21),
        mBuzzer(p22),
        mL1g(p23),
        mL1r(p24),
        mL2g(p25),
        mL2r(p26),
        mPedg(p27),
        mPedr(p28){}

    /**
     * Receive the current state of the traffic lights as
     * an integer
     * @return 0 = Lane 1 green,
     * @return 1 = Lane 2 green,
     * @return 2 = Pedestrian green,
     * @return 3 = all red,
    */
    int getlights(){
        if(mL1g){
            return 0;
        }
        else if(mL2g){
            return 1;
        }
        else if(mPedg){
            return 2;
        }
        else{
            return 3;
        }
    }

    void phaseL1() { 
        setLights(1, 0, 0, 0, 1, 1, 0, 0);
    }

    void phaseL2() {
        setLights(0, 1, 0, 1, 0, 1, 0, 0);
    }

    void phasePed() {
        setLights(0, 0, 1, 1, 1, 0, 0, 1);
    }

    void phaseSOS() {
        setLights(0, 0, 0, 1, 1, 1, 1, 0);
    }

    void standby() {
    setLights(0, 0, 0, 1, 1, 1, 0, 0);
}
};

class Choosestate{
    private:
    Getstate     getstate;
    Triggerevent triggerevent;
    Timeout      safety;
    bool noOne;
    /**priority list*/
    bool priority[2][3] = {0,0,0,
                            0,0,0}; 
    /**car counter*/
    int count = 0;

    public:
    Choosestate(){
    // add some form of initialisation or lower, by conditions
    } // remember to configure destructor
    
    /**
     * @brief alter the waiting priority
     * @param action The chosen alteration of the priority list
     *              - "0": moves priority 2 to 1
     *              - "1": add member to priority 1
     *              - "2": add member to priority 2 
     * @param member The chosen member that is applying the alteration
     *              - "1": L1
     *              - "2": L2
     *              - "3": Ped 
    */
    void changePrior(int action, int member){
        switch(action*10+member){
            case 1:
            case 2:
            case 3:
                /**move priority up (clear)*/
                for(int i=0; i<3;i++){
                    priority[0][i] = priority[1][i];
                    priority[1][i] = 0; 
                }
            break;
            
            case 11:
                //add L1 to p1
                priority[0][0] = 1;
            break;

            case 12:
                /**add L2 to p1*/
                priority[0][1] = 1;
            break;

            case 13:
                /**add ped to p1*/
                priority[0][2] = 1;
            break;

            case 21:
                /**add L1 to p2*/
                priority[1][0] = 1;
            break;

            case 22:
                /**add L2 to p2*/
                priority[1][1] = 1;
            break;

            case 23:
                /**add ped to p2*/
                priority[1][2] = 1;
            break;
        }
    }
    /**
     * @brief check if priority 1 is empty
     * @return true of false
    */
    bool priorEmpty(){
        if(!(priority[0][0]|priority[0][1]|priority[0][2])){
            return true;
        }
        else{
            return false;
        }
    }
    /**
     * @brief Takes current state of system and configures the next state
     * @param windowtime the configured safety window
    */
    void choosestate(int windowtime){
        noOne = true;
        /**check Lane 1 state*/
        if(getstate.isL1Waiting()){
            noOne = false;
            switch(triggerevent.getlights()){
                /** case where car is detected and has green light */
                case 1:
                    /**check if car has passed*/
                    if(getstate.hasL1Passed()){
                        /**check if maximum cars have passed*/
                        if(count<=5){
                            /**If passed cars under maximum, allow another, reset timer and increment count*/
                            safety.attach(&triggerevent, &Triggerevent::standby, windowtime);
                            count++;
                        }
                        else{
                            /**if maximum have passed, turn lights red and reset count*/
                                triggerevent.standby();
                                count = 0;                                
                        }
                    }
                break;

                case 0:
                case 2:
                    /** cases where other lights are green at detection, get priority if dont have*/
                    if(!priority[0][1]){
                        if(priorEmpty()){
                            changePrior(1,2);
                        }
                        else{
                            changePrior(2,2);
                        }
                    }
                break;

                case 3: 
                    /** case where all lights are red, since it means a process has finished,
                     * so begin if have priority one, move priority up, and reset count
                     */
                    if(priority[0][1]){
                        changePrior(0,1);
                        count = 0;
                        triggerevent.phaseL2();
                        safety.attach(&triggerevent, &Triggerevent::standby, windowtime);
                    }
                    
                break;
            }
            }
            if(getstate.isL2Waiting()){
                noOne = false;
                switch(triggerevent.getlights()){
                /** case where car is detected and has green light */
                case 0:
                    /**check if car has passed*/
                    if(getstate.hasL1Passed()){
                        /**check if maximum cars have passed*/
                        if(count<=5){
                            /**If passed cars under maximum, allow another, reset timer and increment count*/
                            safety.attach(&triggerevent, &Triggerevent::standby, windowtime);
                            count++;
                        }
                        else{
                            /**if maximum have passed, turn lights red and reset count*/
                                triggerevent.standby();
                                count = 0;                                
                        }
                    }
                break;

                case 1:
                case 2:
                    /** cases where other lights are green at detection, get priority if dont have*/
                    if(!priority[0][0]){
                        if(priorEmpty()){
                            changePrior(1,1);
                        }
                        else{
                            changePrior(2,1);
                        }
                    }
                break;

                case 3: 
                    /** case where all lights are red, since it means a process has finished,
                     * so begin if have priority one, move priority up, and reset count
                     */
                    if(priority[0][0]){
                        changePrior(0,1);
                        count = 0;
                        triggerevent.phaseL1();
                        safety.attach(&triggerevent, &Triggerevent::standby, windowtime);
                    }
                    
                break;
                }
            }
            if(getstate.isL2Waiting()){
                noOne = false;
                switch(triggerevent.getlights()){
                /** case where car is detected and has green light */
                case 0:
                    /**check if car has passed*/
                    if(getstate.hasL2Passed()){
                        /**check if maximum cars have passed*/
                        if(count<=4){
                            /**If passed cars under maximum, allow another, reset timer and increment count*/
                            safety.attach(&triggerevent, &Triggerevent::standby, windowtime);
                            count++;
                        }
                        else{
                            /**if maximum have passed, turn lights red and reset count*/
                                triggerevent.standby();
                                count = 0;                                
                        }
                    }
                break;

                case 1:
                case 2:
                    /** cases where other lights are green at detection, get priority if dont have*/
                    if(!priority[0][0]){
                        if(priorEmpty()){
                            changePrior(1,2);
                        }
                        else{
                            changePrior(2,2);
                        }
                    }
                break;

                case 3: 
                    /** case where all lights are red, since it means a process has finished,
                     * so begin if have priority one, move priority up, and reset count
                     */
                    if(priority[0][1]){
                        changePrior(0,1);
                        count = 0;
                        triggerevent.phaseL2();
                        safety.attach(&triggerevent, &Triggerevent::standby, windowtime);
                    }
                    
                break;
                }
            }
            if(getstate.isPedWaiting()){
                noOne = false;
                switch(triggerevent.getlights()){
                    case 3:
                    /**pedestrian light is already green*/
                    break;
                    
                    case 0:
                    case 2:
                    /**another light is green*/
                    if(!priority[0][2]){
                        if(priorEmpty()){
                            changePrior(1,3);
                        }
                        else{
                            changePrior(2,3);
                        }
                    }

                }
            }
            if(noOne){
                triggerevent.phaseL1();
            }
    }
};


Choosestate choosest;
int main() {
    while(1){

    }
}