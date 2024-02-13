#include "mbed.h"
#include <ostream>
#include <stdio.h>


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
    bool setState(bool newState) {
        if (state && !newState) {
            flag = true;
            state = newState;
            return flag;
        } else {
            flag = false;
            state = newState;
            return false;
        }
        
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
     * @brief reset flag
    */
    void resetFlag(){
        carFlag.resetFlag();
    }

    /**
     * @brief check if a car has passed L1
     * @return boolean 1 or 0
    */
    bool hasL1Passed(){
        for(int i = 0; i<10; i++){ /**perform multiple checks*/
            if(carFlag.setState(mL1IRclose)){
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
            if(carFlag.setState(mL2IRclose)){
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
        return mL2IRfar;
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
        return 0;
    }
    return 0;
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
    bool waitingFlag;
    Triggerevent():
        mBuzzer(p22),
        mMotor(p21),
        mL1g(p23),
        mL1r(p24),
        mL2g(p25),
        mL2r(p26),
        mPedg(p27),
        mPedr(p28){waitingFlag = false;}

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
        waitingFlag = false;
    }

    void phaseL2() {
        setLights(0, 1, 0, 1, 0, 1, 0, 0);
        waitingFlag = false;
    }

    void phasePed() {
        setLights(0, 0, 1, 1, 1, 0, 0, 1);
        waitingFlag = false;
    }

    void phaseSOS() {
        setLights(0, 0, 0, 1, 1, 1, 1, 0);
        waitingFlag = false;
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
    Timeout      waitTime;
    Serial pc;
    bool noOne;
    /**priority list*/
    bool priority[2][3]; 
    /**car counter*/
    int count;
    /**Default flag*/
    bool dFlag;

    public:
    Choosestate():pc(USBTX, USBRX){
    bool priority[2][3] = {0,0,0,
                           0,0,0};
    count     = 0;
    dFlag     = false;
    noOne     = true;
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
        if(!(priority[0][0]||priority[0][1]||priority[0][2])){
            return true;
        }
        else{
            return false;
        }
    }
    /**
     * @brief Takes current state of system and configures the next state
     * @param windowTimeL1 safety window for L1
     * @param windowTimeL2 safety window for L2
     * @param windowTimePed safety window Ped
    */
    void choosestate(int windowTimeL1, int windowTimeL2, int windowTimePed, int waitL1, int waitL2, int waitPed){
        /**check Lane 1 state*/
        noOne = true;
        for(int i = 0; i<2; i++){
            for(int j = 0; j<3; j++){
                pc.printf("%d,",priority[i][j]);
            }
            pc.printf("\n\r");
        }
        if(getstate.isL1Waiting()||priority[0][0]){
            pc.printf("L1 is waiting \n\r");
            noOne = false;

            switch(triggerevent.getlights()){
                /** case where car is detected and has green light */
                case 0:
                    pc.printf("L1-On\n\r");
                    /**check if car has passed*/
                    if(getstate.hasL1Passed()){
                        /**check if maximum cars have passed*/
                        getstate.resetFlag();
                        pc.printf("L1-Passed\n\r");
                        if(count<=5){
                            /**If passed cars under maximum, allow another, reset timer and increment count*/
                            pc.printf("L1 under max\n\r");
                            safety.attach(&triggerevent, &Triggerevent::standby, windowTimeL1);
                            count++;
                        }
                        else{
                            /**if maximum have passed, turn lights red and reset count*/
                            pc.printf("cars over max\n\r");
                            triggerevent.standby();
                            count = 0;                                
                        }
                    }
                break;

                case 1:
                case 2:
                    /** cases where other lights are green at detection, get priority if dont have*/
                    pc.printf("other light green\n\r");
                    if(dFlag){
                    if((priorEmpty()||priority[0][0])&&!triggerevent.waitingFlag){
                        triggerevent.waitingFlag = true;
                        waitTime.attach(&triggerevent, &Triggerevent::phaseL1, windowTimeL1-waitL1);
                        dFlag = false;
                        changePrior(0,1);
                        safety.attach(&triggerevent, &Triggerevent::standby, windowTimeL1);
                    }
                    else{
                        changePrior(1, 1);
                        }
                    }
                    else{
                        if(!priority[0][0]){
                            if(priorEmpty()){
                                changePrior(1,1);
                            }
                            else{
                                changePrior(2,1);
                            }
                        }
                    }
                break;

                case 3: 
                    /** case where all lights are red, since it means a process has finished,
                     * so begin if have priority one, move priority up, and reset count
                     */
                    pc.printf("all red\n\r");
                    if((priorEmpty())&& !triggerevent.waitingFlag){
                        pc.printf("On with prior empty\n\r");
                        count = 0;
                        triggerevent.waitingFlag = true;
                        waitTime.attach(&triggerevent, &Triggerevent::phaseL1, windowTimeL1-waitL1);
                        dFlag = false;
                        safety.attach(&triggerevent, &Triggerevent::standby, windowTimeL1);
                    }
                    else if((priority[0][0])&&!triggerevent.waitingFlag){
                        pc.printf("On with priority\n\r");
                        changePrior(0,1);
                        count = 0;
                        triggerevent.waitingFlag = true;
                        waitTime.attach(&triggerevent, &Triggerevent::phaseL1, windowTimeL1-waitL1);
                        dFlag = false;
                        safety.attach(&triggerevent, &Triggerevent::standby, windowTimeL1);
                    }
                    
                break;
                default:
                pc.printf("error");
                break;
            }
            }

        if(getstate.isL2Waiting()||priority[0][1]){
            noOne = false;
            pc.printf("L2 waiting\n\r");

            switch(triggerevent.getlights()){
                /** case where car is detected and has green light */
                case 1:
                    /**check if car has passed*/
                    pc.printf("L2 already on\n\r");
                    if(getstate.hasL2Passed()){
                        getstate.resetFlag();
                        /**check if maximum cars have passed*/
                        pc.printf("L2 passed \n\r");
                        if(count<=4){
                            /**If passed cars under maximum, allow another, reset timer and increment count*/
                            pc.printf("cars under max \n\r");
                            safety.attach(&triggerevent, &Triggerevent::standby, windowTimeL2);
                            count++;
                        }
                        else{
                            /**if maximum have passed, turn lights red and reset count*/
                            pc.printf("over max\n\r");
                            triggerevent.standby();
                            count = 0;                                
                        }
                    }
                break;

                case 0:
                case 2:
                    /** cases where other lights are green at detection, get priority if dont have*/
                    pc.printf("other lights on\n\r");
                    if(dFlag){
                        if((priorEmpty()||priority[0][1])&&!triggerevent.waitingFlag){
                            triggerevent.waitingFlag = true;
                            waitTime.attach(&triggerevent, &Triggerevent::phaseL2, windowTimeL2-waitL2);
                            changePrior(0,1);
                            dFlag = false;
                            safety.attach(&triggerevent, &Triggerevent::standby, windowTimeL2);
                        }
                        else{
                            changePrior(1, 2);
                            }
                    }
                    else{
                        if(!priority[0][1]){
                            if(priorEmpty()){
                                changePrior(1,2);
                            }
                            else{
                                changePrior(2,2);
                            }
                        }
                    }
                break;

                case 3: 
                    /** case where all lights are red, since it means a process has finished,
                    * so begin if have priority one, move priority up, and reset count
                    */
                    if((priorEmpty())&&!triggerevent.waitingFlag){
                        count = 0;
                        triggerevent.waitingFlag = true;
                        waitTime.attach(&triggerevent, &Triggerevent::phaseL2, windowTimeL1-waitL2);
                        dFlag = false;
                        safety.attach(&triggerevent, &Triggerevent::standby, windowTimeL2);
                        }
                    else if((priority[0][1])&&!triggerevent.waitingFlag){
                        changePrior(0,1);
                        count = 0;
                        triggerevent.waitingFlag = true;
                            waitTime.attach(&triggerevent, &Triggerevent::phaseL2, windowTimeL1-waitL2);
                        dFlag = false;
                        safety.attach(&triggerevent, &Triggerevent::standby, windowTimeL2);
                    }
                break;
            }
            }
        
        if(getstate.isPedWaiting()||priority[0][2]){
            noOne = false;
            pc.printf("ped waiting \n\r");
            switch(triggerevent.getlights()){

                case 2:
                /**pedestrian light is already green*/
                break;
                
                case 0:
                case 1:
                /**another light is green*/
                pc.printf("another waiting \n\r");
                if(dFlag){
                    if((priorEmpty()||priority[0][2])&&!triggerevent.waitingFlag){
                        triggerevent.waitingFlag = true;
                        waitTime.attach(&triggerevent, &Triggerevent::phasePed, windowTimePed-waitPed);
                        changePrior(0,1);
                        dFlag = false;
                        safety.attach(&triggerevent, &Triggerevent::standby, windowTimePed);
                    }
                    else{
                        changePrior(1, 3);
                        }
                }
                else{
                    if(!priority[0][2]){
                        if(priorEmpty()){
                            changePrior(1,3);
                        }
                        else{
                            changePrior(2,3);
                        }
                    }
                }
                break;

                case 3:
                if((priorEmpty())&&!triggerevent.waitingFlag){
                    triggerevent.waitingFlag = true;
                    waitTime.attach(&triggerevent, &Triggerevent::phasePed, windowTimePed-waitPed);
                    dFlag = false;
                    safety.attach(&triggerevent, &Triggerevent::standby, windowTimePed);
                    }
                else if((priority[0][2]&&!triggerevent.waitingFlag)){
                    changePrior(0,1);
                    triggerevent.waitingFlag = true;
                    waitTime.attach(&triggerevent, &Triggerevent::phasePed, windowTimePed-waitPed);
                    dFlag = false;
                    safety.attach(&triggerevent, &Triggerevent::standby, windowTimePed);
                }
                break;
            }
            }
        
        if(noOne){
            if(triggerevent.getlights()==3 && priorEmpty()&& !triggerevent.waitingFlag&& !dFlag){
            pc.printf("default ");
            waitTime.attach(&triggerevent, &Triggerevent::phaseL1, windowTimeL1-waitL1);
            //safety.attach(&triggerevent, &Triggerevent::phaseL1, windowTimeL2);
            dFlag = true;
            }
        }
    }
};



Choosestate choosest;

int main() {
    while(1){
        choosest.choosestate(3, 3, 3,1,1,1);
        wait(0.5);
    }
}