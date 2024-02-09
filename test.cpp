#include "mbed.h"
#include <stdio.h>

class Interface{ // class that handles overriding and displaying the system
    private:
    Serial mMedium; Getstate *getstate; Triggerevent
    public:
    Interface(){}
    Interface(PinName port1, PinName port2):mMedium(port1,port2){getstate = new Getstate()}
    void display(){
        mMedium.printf("working");
    }
    char override(){
        if(mMedium.readable()){
            return mMedium.getc();
        }
    }
};

class Getstate{
    private:
    DigitalIn mL2IRclose; AnalogIn mL2IRfar; DigitalIn mL1IRclose; AnalogIn mL1IRfar;
    DigitalIn mPedButton;
    public:
    Getstate(){}
    Getstate():mL2IRclose(p20),mL2IRfar(p19),mL1IRclose(p18),mL1IRfar(p17),mPedButton(p16){} 
    // initialise the pins in the constructor
    int isL1Waiting(){
        // add some processing to poll multiple times (maybe using threading)
        if(mL1IRfar==1){ // compute all states as boolean
            return 1;
        }
        else{
            return 0;
        }
    }
    int isL2Waiting(){
        if(mL2IRfar==1){
            return 1;
        }
        else{
            return 0;
        }
    }
    int isPedWaiting(){
        if(mPedButton==1){
            return 1;
        else{
            return 0;
        }
        }
    }
    }
};

class Triggerevent{ // funciton to trigger outputs in accordance with the phase
    private:
    DigitalOut mBuzzer; PwmOut mMotor; DigitalOut mL1g; DigitalOut mL1r;
    DigitalOut mL2g; DigitalOut mL2r; DigitalOut mPedg ;DigitalOut mPedr;
    public:
    Triggerevent(){}
    Triggerevent():mMotor(p21),mBuzzer(p22),mL1g(p23),mL1r(p24),mL2g(p25), mL2r(p26),mPedg(p27),mPedr(p28){}
    void phaseL1(){ // trigger phase L1
        mL1g = 1; mL2g = 0; mPedg = 0;
        Ml1r =0; Ml2r = 1; mPedg = 1;
        mBuzzer = 0; mMotor = 0;
    }
    void phaseL2(){
        mL1g = 0; mL2g = 1; mPedg = 0;
        Ml1r =1; Ml2r = 0; mPedg = 1;
        mBuzzer = 0; mMotor = 0;
    }
    void phasePed(){
        mL1g = 0; mL2g = 0; mPedg = 1;
        Ml1r =1; Ml2r = 1; mPedg = 0;
        mBuzzer = 0; mMotor = 1;
    }
    void phaseSOS(){
        mL1g = 0; mL2g = 0; mPedg = 0;
        Ml1r =1; Ml2r = 1; mPedg = 1;
        mBuzzer = 1; mMotor = 0;
    }
    void standby(){
        mL1g = 0; mL2g = 0; mPedg = 0;
        Ml1r =1; Ml2r = 1; mPedg = 1;
        mBuzzer = 0; mMotor = 0;
    }
};

class Choosestate{
    private:
    Getstate *getstate;
    Triggerevent *triggerevent;
    Timeout safety; 
    public:
    Choosestate(){} 
    Choosestate(){
        getstate = new Getstate();
        triggerevent = new Triggerevent();
        // create priority array
        bool priority[2][3]={0,0,0,
                             0,0,0}; 
        // add some form of initialisation or lower, by conditions
    } // remember to configure destructor

    void choosestate(int windowtime){
        // poll for waiting car
        switch(getstate->isL1waiting()) { 
        case 1:
            // check if l1 already green
            // check current light state
            if(triggerevent->mL2g==1 || triggerevent->mPedg==1){
                // check for main priority
                if(priority[0][0]==1){
                    // if first priority, wait for other light to turn red
                    // change to a function call that keeps checking
                    triggerevent->phaseL1;
                    safety.attach(&triggerevent->standby, windowtime);
                    for 
                }
            }
            else{
                triggerevent->phaseL1;
                safety.attach(&triggerevent->phaseL1);
                for(int i; i=0)
            }
            break;
        case 0:
            break;
        }
        switch(getstate->isL2Waiting()){
            case 1:    
                if(triggerevent->mL2g==1 || triggerevent->mPedg==1){
                    
                }
            break;
        }
        switch(getstate->isPedWaiting()){
            case 1:
                if(triggerevent->mL1g==1 || triggerevent->mL2g==1){

                }

        }
    }
};



int main() {
    Interface interface(USBTX, USBRX); // use USBTX,USBRX for pc, p9,p10 for bluetooth // integrate user input for selecting mode
    // create variable for chosen safety window to pass to functions
    // Test how to check the state of traffic lights
    while(1){
        interface.display();
        wait(0.5);

    }
}