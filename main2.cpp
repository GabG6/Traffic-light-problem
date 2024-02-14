#include "mbed.h"
#include <ostream>
#include <stdio.h>

/*DigitalOut Buzzer(p22);
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
class PedLight; */

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

    /**
     * @brief alter the waiting priority
     * @param action The chosen alteration of the priority list
     *              - "0": moves priority 2 to 1
     *              - "1": add member to priority 1
     *              - "2": add member to priority 2 
     * @param member The chosen member that is applying the alteration
     *              - "0": L1
     *              - "1": L2
     *              - "2": Ped 
    */
    void changePrior(int action, int member){
    switch(action*10+member){
        case 0:
        case 1:
        case 2:
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
        Timeout         safetyTimer;
        Timeout         waitTimer;
        DigitalIn       waitSensor;
        DigitalIn       trafficSensor;
        DigitalOut      greenLight;
        DigitalOut      redLight;
        ManagePrior     managePriority;
        Flag            arriveFlag;
        Flag            leaveFlag;
        int             defaultOn;
        int             safetyTime;
        int             waitTime;
        int             lightNumber;
        bool            (&priority)[2][3];
        bool            isGreenByDefault;
        bool            controlFlag;
        int             maxCounter;
        int             carCounter;
        bool arr[2][3];
        Serial pc;

    public:
        //TrafficLight():isGreenByDefault(1), carCounter(0),controlFlag(0),priority1Flag(0),
        //          priority2Flag(0),waitSensor(P0_0),trafficSensor(P0_0), greenLight(P0_0), redLight(P0_0){}
        TrafficLight():waitSensor(p17),trafficSensor(p18), greenLight(p23), redLight(p24),
                        managePriority(arr), priority(arr), pc(USBTX, USBRX)
                        {isGreenByDefault = 1;lightNumber = 0; safetyTime = 6;waitTime = 2; maxCounter = 5;}
        TrafficLight(int lightNum,bool (&prior)[2][3],int safeTime,int waTime,
                    bool gr_by_def, int maxcount,PinName waitsens,PinName trafficsens,PinName grlight, PinName rdlight): pc(USBTX,USBRX),
                    lightNumber(lightNum),priority(prior),isGreenByDefault(gr_by_def),maxCounter(maxcount),waitSensor(waitsens),
                    trafficSensor(trafficsens), greenLight(grlight), redLight(rdlight),managePriority(prior),safetyTime(safeTime),waitTime(waTime){
                    carCounter=0;controlFlag=0;defaultOn = false;}

        bool update_state(bool otherLFlag, bool pedFlag){
            if(isGreenByDefault && !otherLFlag && !pedFlag && (isPriorEmpty() || isPriority()) && !defaultOn){
                pc.printf("green by default %d \n\r", isGreenByDefault);
                defaultOn = true;
                startGreen();
            } else{
                //defaultOn = false;

                if(carArrived()){
                    pc.printf("car arrived at L%d\n\r", lightNumber);
                    if(isInControl() && carLeft() && (carCounter<maxCounter)){
                        pc.printf("allow another car at L%d, numer %d\n\r", lightNumber, carCounter);
                        carCounter++;
                        resetLeftFlag();
                        resetArriveFlag();
                        startRed();
                    } else{
                        carCounter = 0;
                        if(!otherLFlag && !pedFlag && (isPriorEmpty() || isPriority())){
                            if(isPriority()){
                                clearPriority();
                            }
                            controlFlag = true;
                            resetArriveFlag();
                            startGreen();
                            startRed(); // maybe add an if car left here to only start safety after leaving
                        } else{
                            if(isPriorEmpty()){
                                addPriority1();
                            } else{
                                addPriority2();
                            }
                        }
                        
                    }
                }

            }
        }

        void startRed(){
            safetyTimer.attach(this, &TrafficLight::redLightOn, safetyTime);
        }

        void startGreen(){
            waitTimer.attach(this, &TrafficLight::greenLightOn, waitTime);
        }

        void addPriority2(){
            managePriority.changePrior(2,lightNumber);
        }

        void addPriority1(){
            managePriority.changePrior(1,lightNumber);
        }

        bool isPriorEmpty(){
            return managePriority.priorEmpty();
        }

        bool isPriority(){
            return(managePriority.hasPriority(lightNumber));
        }

        bool isInControl(){
            return controlFlag;
        }

        bool carArrived(){
            return(arriveFlag.carArrive(waitSensor));
        }

        bool carLeft(){

        }

        void resetLeftFlag(){

        }

        void resetArriveFlag(){

        }

        void clearPriority(){
            managePriority.changePrior(0,0);
        }

        void greenLightOn(){
            greenLight = 1;
            redLight   = 0;
        }

        void redLightOn(){
            redLight   = 1;
            greenLight = 0;
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
        int          waitTime;
        int          safetyTime;
        int          lightNumber;
        bool         controlFlag;
        bool         (&priority)[2][3];
    public:
        PedLight(int lightNum, bool (&prior)[2][3],int safeTime, int waTime, PinName grlight,
                PinName rdlight,PinName moto, PinName buzz, PinName butto):
                lightNumber(lightNum),priority(prior),greenLight(grlight),redLight(rdlight), motor(moto),buzzer(buzz),
                button(butto),managePriority(prior),waitTime(waTime),safetyTime(safeTime){
                controlFlag = 0;lightNumber = 2; waitTime = 3;safetyTime = 10;}

        void updateState(bool tl1Flag,bool tl2Flag){
            if(buttonPress() && !tl1Flag && !tl2Flag && (isPriority() || isPriorEmpty())){
                controlFlag = 1;
                if(isPriority()){
                    clearPriority();
                }
                startGreen();
            } else{
                if(isPriorEmpty()){
                    addPriority1();
                } else{
                    addPriority2();
                }
            }
        }

        void greenLightOn(){
            greenLight = 1;
            redLight   = 0;
        }
        
        void redLightOn(){
            redLight    = 1;
            greenLight  = 0;
            controlFlag = false;
        }

        bool isInControl(){
            return controlFlag;
        }

        void clearPriority(){
            managePriority.changePrior(0,0);
        }

        bool isPriority(){
            return(managePriority.hasPriority(lightNumber));
        }
        
        bool isPriorEmpty(){
            return managePriority.priorEmpty();
        }

        void startRed(){
            safetyTimer.attach(this, &PedLight::redLightOn, safetyTime);
        }

        void startGreen(){
            waitTimer.attach(this, &PedLight::greenLightOn, waitTime);
        }

        void addPriority2(){
            managePriority.changePrior(2,lightNumber);
        }

        void addPriority1(){
            managePriority.changePrior(1,lightNumber);
        }
        
        bool buttonPress(){

        }
};

bool priority[2][3] = {0,0,0,
                       0,0,0};
TrafficLight    tl1(0,priority,4,1,1,3,p17,p18,p23,p24);
TrafficLight    tl2(1,priority,4,1,0,3,p19,p20,p25,p26);
PedLight        ped1(2,priority, 12,3,p27,p28,p21,p22,p16);
int main() {
    Serial pc(USBTX,USBRX);
    while(1){
        //tl1.update_state(0, 0);
        tl2.update_state(0,0);
    }
}
