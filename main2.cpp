#include "mbed.h"
#include <ostream>
#include <stdio.h>

DigitalOut Buzzer(p22);
DigitalOut Motor(p21);
//DigitalOut L1g;
DigitalOut L1r(p24);
DigitalOut L2g(p25);
DigitalOut L2r(p26);
DigitalOut Pedg(p27);
DigitalOut Pedr(p28);
DigitalIn L2IRTraffic(p20);
DigitalIn L2IRWait(p19);
DigitalIn L1IRTraffic(p18);
DigitalIn L1IRWait(p17);
DigitalIn PedButton(p16);
class TrafficLight;
class PedLight;

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

class ManagePrior{
    private:
    bool (&priority)[2][3];
    public:
    ManagePrior(bool (&prior)[2][3]):priority(prior){}

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
        case 10:
            //add L1 to p1
            priority[0][0] = 1;
        break;
        case 11:
            /**add L2 to p1*/
            priority[0][1] = 1;
        break;
        case 12:
            /**add ped to p1*/
            priority[0][2] = 1;
        break;
        case 20:
            /**add L1 to p2*/
            priority[1][0] = 1;
        break;
        case 21:
            /**add L2 to p2*/
            priority[1][1] = 1;
        break;
        case 22:
            /**add ped to p2*/
            priority[1][2] = 1;
        break;
    }
    }
    bool hasPriority(int member){
        if(priority[0][member]){
            return true;
        }
    }
    bool priorEmpty(){
        if(!(priority[0][0]||priority[0][1]||priority[0][2])){
            return true;
        }
        else{
            return false;
        }
    }
};

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

/**
 * @class for controlling the state of the traffic lights
 * @param prior priority array
 * @param safetytime safety time
 * @param waittime wait for green time
 * @param isgreenbydefault choose if this is the main road
 * @param carcount maximum car count
 * @param waitsens wait sensor pin
 * @param trafficsensor traffic sensor pin
 * @param greenlight green light pin
 * @param redlight red light pin
*/
class TrafficLight{
private:
    Timeout         safety;
    Timeout         waitTime;
    DigitalIn       waitSensor;
    DigitalIn       trafficSensor;
    DigitalOut      greenLight;
    DigitalOut      redLight;
    ManagePrior     managePriority;
    int             lightNumber;
    bool            (&priority)[2][3];
    bool            isGreenByDefault;
    bool            controlFlag;
    int             carCounter;

public:
    //TrafficLight():isGreenByDefault(1), carCounter(0),controlFlag(0),priority1Flag(0),
      //          priority2Flag(0),waitSensor(P0_0),trafficSensor(P0_0), greenLight(P0_0), redLight(P0_0){}
    TrafficLight(int lightNum,bool (&prior)[2][3],int safetyTime,int waitTime,
                bool gr_by_def, int carcount,PinName waitsens,PinName trafficsens,PinName grlight, PinName rdlight):
                lightNumber(lightNum),priority(prior),isGreenByDefault(gr_by_def), carCounter(carcount),
                waitSensor(waitsens),trafficSensor(trafficsens), greenLight(grlight), redLight(rdlight){
                isGreenByDefault=1; carCounter=0;controlFlag=0;waitSensor=P0_0;
                trafficSensor=P0_0; greenLight=P0_0; redLight=P0_0;lightNumber = 0;}

    bool isInControl(){
        return controlFlag;
    }

    bool carArrived(){

    }

    bool carLeft(){

    }

    bool update_state(bool othLightFlag, bool pedFlag){
        
    }

    void greenLightOn(){
        greenLight = 1;
    }

    void redLightOn(){
        redLight = 1;
        controlFlag = false;
    }
};

class PedLight{
private:
    Timeout      safetyTimer;
    Timeout      waitTimer;
    DigitalOut   greenLight;
    DigitalOut   redLight;
    DigitalOut   motor;
    DigitalOut   buzzer;
    DigitalIn    button;
    ManagePrior  managePriority;
    int          lightNumber;
    bool         controlFlag;
    bool         (&priority)[2][3];
public:
    PedLight(int lightNum, bool (&prior)[2][3],int safetyTime, int waitTime, PinName grlight,
            PinName rdlight,PinName moto, PinName buzz, PinName butto):
            lightNumber(lightNum),priority(prior),greenLight(grlight),redLight(rdlight), motor(moto),buzzer(buzz),
            button(butto),managePriority(priority){
            controlFlag = 0; greenLight = P0_0; redLight = P0_0;
            motor =P0_0; buzzer =P0_0, button = P0_0;lightNumber = 2;}

    bool isInControl(){
        return controlFlag;
    }

    void updateState(bool tl1Flag,bool tl2Flag){
        if(isInControl()){
            if()
        }
    }

    void greenLightOn(){
        greenLight = 1;
    }
    void redLightOn(){
        controlFlag = false;
    }
    
};
bool priority[2][3] = {0,0,0,
                       0,0,0};
TrafficLight    tl1(0,priority,5,5,true,5,p17,p18,p23,p24);
TrafficLight    tl2(1,priority,5,5,false,4,p19,p20,p25,p26);
PedLight        ped1(2,priority, 12,3,p27,p28,p21,p22,p16);
int main() {
    Serial pc(USBTX,USBRX);

}
