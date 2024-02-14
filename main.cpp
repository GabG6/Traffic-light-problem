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
    bool hasPriority(int num,int member){
        switch (num)
        {
        case 1:
            if(priority[0][member]){
                return true;
            } else{
                return false;
        }
            break;
        
        case 2:
            if(priority[1][member]){
                return true;
            } else{
                return false;
        }
            break;
        }
        
    }
    bool priorEmpty(int num){
        switch (num){
        case 1:
            if(!(priority[0][0]||priority[0][1]||priority[0][2])){
                return true;
            }
            else{
                return false;
            }
        break;

        case 2:
            if(!(priority[1][0]||priority[1][1]||priority[1][2])){
                return true;
            }
            else{
                return false;
            }
        break;
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
        if(!flag){
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
    }

    /**
     * @brief Compute a change from high to low
     * @param sensor The current state of the sensor
     * @return The flag of the state change*/ 
    bool carLeft(bool sensor){
        if(!flag){
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
 * @param lightnum the light number in the junction
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
            if(isGreenByDefault && !otherLFlag && !pedFlag && (isPriorEmpty() || isPriority1()) && isPriorEmpty() && isPrior2Empty()){
                controlFlag = true;
                if(isPriority1()){
                    clearPriority();
                    pc.printf("priority cleared at default\n\r");
                }
                if(!defaultOn){
                    pc.printf("Default on\n\r;");
                    resetArriveFlag();
                    defaultOn = true;
                    startGreen();
                } else{
                    startRedDefault();
                }
            } else{
                defaultOn = false;

                if(hasCarArrived()){
                    if(isInControl() && hasCarLeft() && ((carCounter<=maxCounter) || (isPriorEmpty() && isPrior2Empty()) && !(isPriority2() || isPriority1()))){
                        pc.printf("allow another car at L%d, number %d,\n\r", lightNumber+1, carCounter);
                        if(isPriority1()){
                            clearPriority();
                            pc.printf("priority cleared at control\n\r");
                        }
                        carCounter++;
                        if(isPriorEmpty()){
                            if(carCounter==maxCounter){
                                carCounter--;
                        }
                        }
                        resetLeftFlag();
                        resetArriveFlag();
                        startRed();
                    } else if(!otherLFlag && !pedFlag && (isPriorEmpty() || isPriority1()) && !isInControl() && !isPriority2()){
                            carCounter = 0;
                            pc.printf("On by red for L%d\n\r", lightNumber+1);
                            if(isPriority1()){
                                clearPriority();
                                pc.printf("priority cleared at red\n\r");
                            }
                            controlFlag = true;
                            resetArriveFlag();
                            startGreen();
                            startRed(); // maybe add an if car left here to only start safety after leaving
                        } else if(!isInControl() || isPriorEmpty() || carCounter>maxCounter){
                            if(isPriorEmpty()){
                                carCounter = 0;
                                pc.printf(" adding L%d to priority 1\n\r", lightNumber+1);
                                addPriority1();
                            }
                            else if(!isPriority1() && !isPriority2() && isPrior2Empty()){
                                pc.printf(" adding L%d to priority 2\n\r", lightNumber+1);
                                addPriority2();
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

        void startRedDefault(){
            safetyTimer.attach(this, &TrafficLight::redLightOn, waitTime);
        }

        void addPriority2(){
            managePriority.changePrior(2,lightNumber);
        }

        void addPriority1(){
            managePriority.changePrior(1,lightNumber);
        }

        bool isPriorEmpty(){
            return managePriority.priorEmpty(1);
        }

        bool isPrior2Empty(){
            return managePriority.priorEmpty(2);
        }

        void clearPriority(){
            managePriority.changePrior(0,0);
        }

        bool isPriority1(){
            return(managePriority.hasPriority(1,lightNumber));
        }
        bool isPriority2(){
            return(managePriority.hasPriority(2,lightNumber));
        }

        bool isInControl(){
            return controlFlag;
        }

        bool hasCarArrived(){
            return(arriveFlag.carArrive(waitSensor));
        }

        bool hasCarLeft(){
            return(leaveFlag.carLeft(waitSensor));
        }

        void resetLeftFlag(){
            arriveFlag.resetFlag();
        }

        void resetArriveFlag(){
            arriveFlag.resetFlag();
        }

        void greenLightOn(){
            greenLight = 1;
            redLight   = 0;
        }

        void redLightOn(){
            redLight   = 1;
            greenLight = 0;
            controlFlag = false;
            carCounter = 0;
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
        Flag         buttonFlag;
        int          waitTime;
        int          safetyTime;
        int          lightNumber;
        bool         controlFlag;
        bool         (&priority)[2][3];
        Serial       pc;

    public:
        PedLight(int lightNum, bool (&prior)[2][3],int safeTime, int waTime, PinName grlight,
                PinName rdlight,PinName moto, PinName buzz, PinName butto):pc(USBTX, USBRX),
                lightNumber(lightNum),priority(prior),greenLight(grlight),redLight(rdlight), motor(moto),buzzer(buzz),
                button(butto),managePriority(prior),waitTime(waTime),safetyTime(safeTime){
                controlFlag = 0;lightNumber = 2; waitTime = 3;safetyTime = 10;}

        void updateState(bool tl1Flag,bool tl2Flag){
            if(buttonPress()){
                if(!tl1Flag && !tl2Flag && (isPriorEmpty() || isPriority1()) && !isInControl() && !isPriority2()){
                    pc.printf("Pedestrian on\n\r");
                    if(isPriority1()){
                        clearPriority();
                        pc.printf("priority cleared at red\n\r");
                    }
                    resetButton();
                    controlFlag = true;
                    startGreen();
                    startRed();
                } else if(!isInControl() || isPriorEmpty()){
                    if(isPriorEmpty()){
                        pc.printf("adding ped to priority 1\n\r");
                        addPriority1();
                    }
                    else if(!isPriority1() && !isPriority2() && isPrior2Empty()){
                        pc.printf("adding ped to priority 2\n\r");
                        addPriority2();
                    }
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

        bool isPriority1(){
            return(managePriority.hasPriority(1,lightNumber));
        }
        bool isPriority2(){
            return(managePriority.hasPriority(2,lightNumber));
        }
        
        bool isPriorEmpty(){
            return managePriority.priorEmpty(1);
        }

        bool isPrior2Empty(){
            return managePriority.priorEmpty(2);
        }

        void addPriority2(){
            managePriority.changePrior(2,lightNumber);
        }

        void addPriority1(){
            managePriority.changePrior(1,lightNumber);
        }

        void startRed(){
            safetyTimer.attach(this, &PedLight::redLightOn, safetyTime);
        }

        void startGreen(){
            waitTimer.attach(this, &PedLight::greenLightOn, waitTime);
        }
        
        bool buttonPress(){
            return(buttonFlag.carArrive(button));
        }

        void resetButton(){
            buttonFlag.resetFlag();
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
        tl1.update_state(tl2.isInControl(), ped1.isInControl());
        tl2.update_state(tl1.isInControl(),ped1.isInControl());
        ped1.updateState(tl1.isInControl(), tl2.isInControl());
    }
}
