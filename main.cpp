#include "mbed.h"
#include <stdio.h>

class Interface{ // class that handles overriding and displaying the system
    private:
    Serial mMedium;
    public:
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
    Timeout safetyL1; Timeout safetyL2; Timeout safetyPed;
    public:
    Triggerevent(){}
    Triggerevent():mMotor(p21),mBuzzer(p22),mL1g(p23),mL1r(p24),mL2g(p25), mL2r(p26),mPedg(p27),mPedr(p28){}
    void phaseL1(){ // trigger phase L1
        mL1g = 1; mL2g = 0; mPedg = 0;
        Ml1r =0; Ml2r = 1; mPedg = 1;
        mBuzzer = 0; mMotor = 0;
    }
    void phaseL2


};

class Choosestate{
    private:
    Getstate *getstate;
    public:
    Choosestate(){} 
    Choosestate(){getstate = new Getstate();} // remember to configure destructor
    int choosestate(){
        switch (getstate->isL1waiting()) {
        case 0:
            return 0;
            break;
        }
    }
};



int main() {
    Getstate getst;
    Interface interface(USBTX, USBRX); // use USBTX,USBRX for pc, p9,p10 for bluetooth // integrate user input for selecting mode
    while(1){
        interface.display();
        wait(0.5);
        //pc.printf("Choice: %d", getst.choosestate());
        /*int ch = getst.choosestate();
        switch (ch) {
        case 0:
            l1g=1;
            l1r=0;
            l2g=0;
            wait(0.2);
            break;
        case 1:
            l1g=0;
            l1r=1;
            l2g=0;
            wait(0.2);
            break;
        case 2:
            l1g=0;
            l1r=0;
            l2g=1;
            wait(0.2);
            break;
        }*/
    }
}