#include "mbed.h"
#include <stdio.h>


class Getstate{
    private:
        DigitalIn mL2IRclose;
        DigitalIn mL2IRfar;
        DigitalIn mL1IRclose;
        DigitalIn mL1IRfar;
        DigitalIn mPedButton;
    
    public:
        Getstate():
        mL2IRclose(p20),
        mL2IRfar(p19),
        mL1IRclose(p18),
        mL1IRfar(p17),
        mPedButton(p16){}
    /**
     * @brief check if a car has passed L1
    */
    bool hasL1Passed(){
        int averagecount = 0;
        for(int i=0; i<10; i++){
            if(mL1IRclose==1){
                averagecount++;
            }
        }
        return (averagecount>=7);
    }
    /**
     * @brief check if a car has passed L2
    */
    bool hasL2Passed(){
        int averagecount = 0;
        for(int i=0; i<10; i++){
            if(mL2IRclose==1){
                averagecount++;
            }
        }
        return (averagecount>=7);
    }
    /**
     * @brief check if a car is waiting L1
    */
    bool isL1Waiting(){
        int averagecount = 0;
        for(int i=0; i<10; i++){
            if(mL1IRfar==1){
                averagecount++;
            }
        }
        return (averagecount>=7);
        }
    /**
     * @brief check if a car is waiting L2
    */
    bool isL2Waiting(){
        int averagecount = 0;
        for(int i=0; i<10; i++){
            if(mL2IRfar==1){
                averagecount++;
            }
        }
        return (averagecount>=7);
        }
        }
    /**
     * @brief check if a pedestrian is waiting
    */
    bool isPedWaiting(){
        for(int i=0; i<10; i++){
            if(mPedButton==1){
                return 1;
            }
            else{
                return 0;
            }
        }
        }
};

class Triggerevent{ // funciton to trigger outputs in accordance with the phase
    private:
        DigitalOut mBuzzer;
        PwmOut mMotor;
        DigitalOut mL1g;
        DigitalOut mL1r;
        DigitalOut mL2g;
        DigitalOut mL2r;
        DigitalOut mPedg;
        DigitalOut mPedr;
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
        if(mL1g==1){
            return 0;
        }
        else if(mL2g==1){
            return 1;
        }
        else if(mPedg==1){
            return 2;
        }
        else{
            return 3;
        }
    }

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
    Getstate *getstate;
    Triggerevent *triggerevent;
    Timeout safety; 
    public:
    Choosestate(){
        getstate = new Getstate();
        triggerevent = new Triggerevent();
        // create priority array
        bool priority[2][3]={0,0,0,
                             0,0,0}; 
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
        else{
            switch(action*10+member){
                case 1:

            }
        }

    }
    /**
     * @brief Takes current state of system and configures the next state
     * @param windowtime the configured safety window
    */
    void choosestate(int windowtime){
        if(getstate->isL1Waiting()){
            switch(triggerE->getlights()){ // check current light stat
                case 0:
                    /** case where car is detected and has green light */

                    break;
                case 1:
                case 2:
                    /** cases where other lights are green at detection, wait for priority*/

                    break;
                case 3: 
                    /** case where all lights are red, since it means a process has finished, simply begin */
                    triggerE->phaseL1;
                    
                    break;
            }
                if(priority[0][0]==1){
                    // if first priority, wait for other light to turn red
                    // change to a function call that keeps checking
                    triggerevent->phaseL1;
                    safety.attach(&triggerevent->standby, windowtime);
                    
                }
            }
            else{
                triggerevent->phaseL1;
                safety.attach(&triggerevent->phaseL1);
                for(int i; i=0)
            }
        }
        if(getstate->isL2Waiting()){
  
            if(triggerevent->mL2g==1 || triggerevent->mPedg==1){   
            }
        }
        if(getstate->isPedWaiting()){
            if(triggerevent->mL1g==1 || triggerevent->mL2g==1){
            }
        }
    }
};

Triggerevent triggerE;
Choosestate choosest;
int main() {
    Interface interface(USBTX, USBRX); // use USBTX,USBRX for pc, p9,p10 for bluetooth // integrate user input for selecting mode
    // create variable for chosen safety window to pass to functions
    // Test how to check the state of traffic lights
    while(1){
        choose.choosestate()
        
        getst.isL1Waiting==1 ? triggerE.phaseL1 : 0;
        getst.isL2Waiting==1 ? triggerE.phaseL2 : 0;
        getst.isPedWaiting==1 ? triggerE.phasePed:0;

    }
    triggerE.getlights();
    choosest.changePrior();
}