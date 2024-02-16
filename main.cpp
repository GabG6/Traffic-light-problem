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
 * @brief Manage the priority matrix
 * 
 * The class takes a reference to the global priority matrix, and it adds members to it,
 * checks if it's empty, and moves the priority up. This class is key for expandability,
 * in order to add members to the junction, a new row and column in the priority matrix 
 * needs to be implemented. This entails changing the size of the parameter, adding the 
 * cases in the changePrior, hasPriority, and priorEmpty functions
 * 
 * @param priority refernece to the global priority matrix 
*/
class ManagePrior{
    private:
    /// @brief reference to priority matrix
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
                ///move priority up (clear)
                for(int i=0; i<3;i++){
                    priority[0][i] = priority[1][i];
                    priority[1][i] = 0; 
                }
            break;
            case 10:
                ///add L1 to p1
                priority[0][0] = 1;
            break;
            case 11:
                ///add L2 to p1
                priority[0][1] = 1;
            break;
            case 12:
                ///add ped to p1
                priority[0][2] = 1;
            break;
            case 20:
                ///add L1 to p2
                priority[1][0] = 1;
            break;
            case 21:
                ///add L2 to p2
                priority[1][1] = 1;
            break;
            case 22:
                ///add ped to p2
                priority[1][2] = 1;
            break;
        }
    }
    /**
     * @brief check if the member has priority1 or priority 2
     * 
     * @param num the priority number to check
     * @param  member the chosen member to check
     * @return boolean value of the priority slot
    */
    bool hasPriority(int num,int member){
        switch (num)
        {
        case 1:
            ///check if member has priority 1 
            if(priority[0][member]){
                return true;
            } else{
                return false;
        }
            break;
        
        case 2:
            /// check if member has priority 2
            if(priority[1][member]){
                return true;
            } else{
                return false;
        }
            break;
        }
        
    }
    /**
     * @brief check if priority is empty
     * @param num the member to check
     * @return boolean value of the priority row  
    */
    bool priorEmpty(int num){
        switch (num){
        case 1:
            ///check if priority row 1 is empty
            if(!(priority[0][0]||priority[0][1]||priority[0][2])){
                return true;
            }
            else{
                return false;
            }
        break;

        case 2:
            ///check if priority row 2 is empty
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
 * @brief Implementation of sensor Flags
 * 
 * The class computes the rising and falling edge of sensor inputs, storing previous values of
 * a sensor input, and comparing it to the previous value. The value is only computed if the flag
 * is false, so the flag remains true until explicitly reset according to desired conditions
 * 
*/
class Flag{
    private:
    /// @brief the previous signal
    bool prevState;
    /// @brief the signalling flag
    bool flag;
    public:
    Flag(): prevState(false), flag(false){}

    /**
     * @brief Compute a change from low to high
     * @param sensor The current state of the sensor
     * @return The flag of the state change
    */    
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
     * @return The flag of the state change
    */ 
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
 * 
 * The class is initiated for every car traffic light member in the system, it takes
 * the pin numbers of the microcontroller as parameters, relating to the pins of the 
 * sensors and the lights. The Class uses a controlFlag that is passed around across traffic
 * light objects, to represent which traffic light is currently in control.
 * When not in control, the objects manage the shared priority matix, based on a first come
 * first served basis. Once a light is no longer in control, which is when it turns to red,
 * control is passed to the 1st priority light, which then clears its priority. 
 * The class allows a specific number of cars to pass, or a specific amount of time, based on 
 * parameters, after which it adds itself to the queue and passes control.
 * 
 * For adding or removing members in the system, the control flag variables in the main function
 * need to be adjusted to match the members in the junction
 * 
 * @param lightnum the light number in the junction, mostly for console log
 * @param prior reference to priority array
 * @param safetyTime safety time in seconds
 * @param waitTime wait for green time
 * @param isgreenbydefault choose if this is the main road
 * @param carcount maximum car count
 * @param waitsens wait sensor pin
 * @param trafficsensor traffic sensor pin
 * @param greenlight green light pin
 * @param redlight red light pin
 * 
*/
class TrafficLight{
    private:
        /// @brief Timeout object for the safety time
        Timeout         safetyTimer;
        /// @brief Timeout object for the wait time
        Timeout         waitTimer;
        /// @brief Waiting sensor object
        DigitalIn       waitSensor;
        /// @brief Traffic sensor object, if system is reconfigured to 2 sensors
        DigitalIn       trafficSensor;
        /// @brief Green light output 
        DigitalOut      greenLight;
        /// @brief Red light output 
        DigitalOut      redLight;
        /// @brief Priority manager 
        ManagePrior     managePriority;
        /// @brief Flag manager for rise signal
        Flag            arriveFlag;
        /// @brief Flag manager for fall signal
        Flag            leaveFlag;
        /// @brief Is default road on Flag
        int             defaultOn;
        /// @brief Safety time in seconds
        int             safetyTime;
        /// @brief Wait time in seconds
        int             waitTime;
        /// @brief Identifier number of light
        int             lightNumber;
        /// @brief Reference to global priority matrix
        bool            (&priority)[2][3];
        /// @brief Main road identifier
        bool            isGreenByDefault;
        /// @brief Has control flag
        bool            controlFlag;
        /// @brief Max cars to pass in one sequence
        int             maxCounter;
        /// @brief Current sequence car counter
        int             carCounter;
        /// @brief Default priority
        bool            arr[2][3];
        /// @brief Console log object
        Serial          pc;
        /**
         * @brief Begin red light timeout
        */
        void startRed(){
            safetyTimer.attach(this, &TrafficLight::redLightOn, safetyTime);
        }
        /**
         * @brief Begin green light timeout
        */
        void startGreen(){
            waitTimer.attach(this, &TrafficLight::greenLightOn, waitTime);
        }
        /**
         * @brief Begin default red light timeout, with a shorter timer
        */
        void startRedDefault(){
            safetyTimer.attach(this, &TrafficLight::redLightOn, waitTime);
        }
        /**
         * @brief add member to priority 2
        */
        void addPriority2(){
            managePriority.changePrior(2,lightNumber);
        }
        /**
         * @brief add member to priority 1
        */
        void addPriority1(){
            managePriority.changePrior(1,lightNumber);
        }
        /**
         * @brief check if priority 1 is empty
         * @return boolean is row empty
        */
        bool isPrior1Empty(){
            return managePriority.priorEmpty(1);
        }
        /**
         * @brief check if priority 2 is empty
         * @return boolean is row empty
        */
        bool isPrior2Empty(){
            return managePriority.priorEmpty(2);
        }
        /**
         * @brief move priority 2 to 1
        */
        void clearPriority(){
            managePriority.changePrior(0,0);
        }
        /**
         * @brief check if member has priority 1
         * @return boolean of priority
        */
        bool isPriority1(){
            return(managePriority.hasPriority(1,lightNumber));
        }
        /**
         * @brief check if member has priority 2
         * @return boolean of priority
        */
        bool isPriority2(){
            return(managePriority.hasPriority(2,lightNumber));
        }
        /**
         * @brief check if car arrive flag is on
         * @return boolean flag
        */
        bool hasCarArrived(){
            return(arriveFlag.carArrive(!waitSensor));
        }
        /**
         * @brief check if car left flag is on
         * @return boolean flag
        */
        bool hasCarLeft(){
            return(leaveFlag.carLeft(!waitSensor));
        }
        /**
         * @brief reset has car left flag
        */
        void resetLeftFlag(){
            leaveFlag.resetFlag();
        }
        /**
         * @brief reset has car arrived flag
        */
        void resetArriveFlag(){
            arriveFlag.resetFlag();
        }

    public:
        TrafficLight():waitSensor(p17),trafficSensor(p18), greenLight(p23), redLight(p24),
                        managePriority(arr), priority(arr), pc(USBTX, USBRX)
                        {isGreenByDefault = 1;lightNumber = 0; safetyTime = 6;waitTime = 2; maxCounter = 5;carCounter=0;controlFlag=0;defaultOn = false;}
        TrafficLight(int lightNum,bool (&prior)[2][3],int safeTime,int waTime,
                    bool gr_by_def, int maxcount,PinName waitsens,PinName trafficsens,PinName grlight, PinName rdlight): pc(USBTX,USBRX),
                    lightNumber(lightNum),priority(prior),isGreenByDefault(gr_by_def),maxCounter(maxcount),waitSensor(waitsens),
                    trafficSensor(trafficsens), greenLight(grlight), redLight(rdlight),managePriority(prior),safetyTime(safeTime+waTime),waitTime(waTime){
                    carCounter=0;controlFlag=0;defaultOn = false;}
        /**
         * @brief Main function that computes the priority and state
         * 
         * The function uses the state of the other lights' and the priority matrix to decide if the traffic light
         * should be in or if it should wait for the other members first.
         * 
         * @param otherLFlag Control flag of other car traffic light
         * @param pedFlag Control flag of pedestrian crossing
         * 
        */
        void update_state(bool otherLFlag, bool pedFlag){
            if(isGreenByDefault && !otherLFlag && !pedFlag && (isPrior1Empty() || isPriority1()) && isPrior1Empty() && isPrior2Empty()){
                ///Keep main road on if no one waiting
                controlFlag = true;
                if(isPriority1()){
                    ///Clear the priority if member has it
                    clearPriority();
                    pc.printf("priority cleared at default\n\r");
                }
                if(!defaultOn){
                    ///Turn main road on if not on
                    pc.printf("Default on\n\r;");
                    resetArriveFlag();
                    defaultOn = true;
                    startGreen();
                } else{
                    ///If main road already on, reset Red Tout to keep it on
                    startRedDefault();
                }
            } else{
                ///Turn off is main road on flag
                defaultOn = false;

                if(hasCarArrived()){
                    ///Logic if a car is waiting at junction
                    if(isInControl() && hasCarLeft() && ((carCounter<=maxCounter) || (isPrior1Empty() && isPrior2Empty()) && !(isPriority2() || isPriority1()))){
                        ///If member has green light and no one is waiting, or under max cars, allow another car
                        pc.printf("allow another car at L%d, number %d,\n\r", lightNumber+1, carCounter+1);
                        if(isPriority1()){
                            ///Clear priority if has it
                            clearPriority();
                            pc.printf("priority cleared at control\n\r");
                        }
                        carCounter++;
                        if(isPrior1Empty()){
                            if(carCounter==maxCounter){
                                ///If no one is waiting, decrement counter so that only one more car is allowed once a member is waiting
                                carCounter--;
                        }
                        }
                        ///Reset flags and start red to reset timer
                        resetLeftFlag();
                        resetArriveFlag();
                        startRed();
                    } else if(!isInControl()&& !otherLFlag && !pedFlag && (isPrior1Empty() || isPriority1()) && !isInControl() && !isPriority2()){
                            ///If other members dont have control and member has priority, member can turn on 
                            pc.printf("On by red for L%d\n\r", lightNumber+1);
                            ///Reset car counter
                            carCounter = 0;
                            if(isPriority1()){
                                ///Clear priority if has it
                                clearPriority();
                                pc.printf("priority cleared at red\n\r");
                            }
                            ///Grant control and reset flags
                            controlFlag = true;
                            resetArriveFlag();
                            startGreen();
                            startRed();
                        } else if(!isInControl()){
                            ///Is waiting for other members
                            if(isPrior1Empty()){
                                ///First one waiting, add to priority 1
                                pc.printf(" adding L%d to priority 1\n\r", lightNumber+1);
                                ///Counter reset for adding car after maximum to queue 
                                
                                addPriority1();
                            }
                            else if(!isPriority1() && !isPriority2() && isPrior2Empty()){
                                ///Second one waiting, add to priority 2
                                pc.printf(" adding L%d to priority 2\n\r", lightNumber+1);
                                addPriority2();
                            }
                        }  
                }

            }
        }
        /**
         * @brief Check if member is in control
         * @return Boolean value of control
        */
        bool isInControl(){
            return controlFlag;
        }
        /**
         * @brief green on red off
        */
        void greenLightOn(){
            greenLight = 1;
            redLight   = 0;
        }
        /**
         * @brief green off red on, control flag off, reset car counter
        */
        void redLightOn(){
            redLight   = 1;
            greenLight = 0;
            controlFlag = false;
            carCounter = 0;
        }
};
/**
 * @brief class to compute the state of the pedestrian light
 * 
 * Refer to class @class TrafficLight, slight logic changes, no default state, no counter
 * @param Lightnum light number identifier
 * @param prior reference to priority array
 * @param safeTime the safety time in seconds
 * @param waittime the wait time in seconds
 * @param greenlight green light pin name
 * @param redlight red light pin name
 * @param motor motor pin name
 * @param buzzer buzzer pin name
 * @param button button pin name
*/
class PedLight{
    private:
        /// @brief Timeout object for the safety time
        Timeout      safetyTimer;
        /// @brief Timeout object for the wait time
        Timeout      waitTimer;        
        /// @brief Green light output         
        DigitalOut   greenLight;
        /// @brief Red light output 
        DigitalOut   redLight;
        /// @brief motor output
        DigitalOut   motor;
        /// @brief buzzer output
        DigitalOut   buzzer;
        /// @brief button input
        DigitalIn    button;
        /// @brief priority manager
        ManagePrior  managePriority;
        /// @brief button rise detector
        Flag         buttonFlag;
        /// @brief wait time in seconds
        int          waitTime;
        /// @brief safety time in seconds
        int          safetyTime;
        /// @brief identifier number of light
        int          lightNumber;
        /// @brief member has control flag
        bool         controlFlag;
        /// @brief reference to gloval priority matrix
        bool         (&priority)[2][3];
        /// @brief console log
        Serial       pc;
    
        /**
         * @brief function to move the priority up
        */
        void clearPriority(){
            managePriority.changePrior(0,0);
        }
        /**
         * @brief check if the member has priority 1
         * @return boolean value of priority 1
        */
        bool isPriority1(){
            return(managePriority.hasPriority(1,lightNumber));
        }
        /**
         * @brief check if the member has priority 2
         * @return boolean value of priority 2
        */
        bool isPriority2(){
            return(managePriority.hasPriority(2,lightNumber));
        }
        /**
         * @brief check if the priority one slots are empty
         * @return boolean value for priority slot
        */
        bool isPrior1Empty(){
            return managePriority.priorEmpty(1);
        }
        /**
         * @brief check if the priority 2 slots are open
         * @return boolean value for the slot
        */
        bool isPrior2Empty(){
            return managePriority.priorEmpty(2);
        }
        /**
         * @brief add member to priority 2 
        */
        void addPriority2(){
            managePriority.changePrior(2,lightNumber);
        }
        /**
         * @brief add member to priority 1
        */
        void addPriority1(){
            managePriority.changePrior(1,lightNumber);
        }
        /**
         * @brief begin the red light timeout sequence
        */
        void startRed(){
            safetyTimer.attach(this, &PedLight::redLightOn, safetyTime);
        }
        /**
         * @brief begin the green light timeout sequence
        */
        void startGreen(){
            waitTimer.attach(this, &PedLight::greenLightOn, waitTime);
        }
        /**
         * @brief check for rise in the pedestrian button signal
         * @return boolean flag of the button press
        */
        bool buttonPress(){
            return(buttonFlag.carArrive(button));
        }
        /**
         * @brief reset the flag for the pedestrian button flag
        */
        void resetButton(){
            buttonFlag.resetFlag();
        }

    public:
        PedLight(int lightNum, bool (&prior)[2][3],int safeTime, int waTime, PinName grlight,
                PinName rdlight,PinName moto, PinName buzz, PinName butto):pc(USBTX, USBRX),
                lightNumber(lightNum),priority(prior),greenLight(grlight),redLight(rdlight), motor(moto),buzzer(buzz),
                button(butto),managePriority(prior),waitTime(waTime),safetyTime(safeTime+waTime){
                controlFlag = 0;lightNumber = 2; waitTime = 3;safetyTime = 10;}

        void updateState(bool tl1Flag,bool tl2Flag){
            if(buttonPress()){
                ///Button press deected
                if(!tl1Flag && !tl2Flag && (isPrior1Empty() || isPriority1()) && !isInControl() && !isPriority2()){
                    ///If other members not in control and has priority/priority empty
                    pc.printf("Pedestrian on\n\r");
                    if(isPriority1()){
                        ///Clear priority if has it
                        clearPriority();
                        pc.printf("priority cleared at red\n\r");
                    }
                    ///Reset flag and control true
                    resetButton();
                    controlFlag = true;
                    ///begin green and red timeouts
                    startGreen();
                    startRed();
                } else if(!isInControl() || isPrior1Empty()){
                    ///Other is in control or no main priority
                    if(isPrior1Empty()){
                        ///Add to priority 1 if empty
                        pc.printf("adding ped to priority 1\n\r");
                        addPriority1();
                    }
                    else if(!isPriority1() && !isPriority2() && isPrior2Empty()){
                        ///Add to priority 2 is empty
                        pc.printf("adding ped to priority 2\n\r");
                        addPriority2();
                    }
                }
            }
        }
        /**
         * @brief green on red off
        */
        void greenLightOn(){
            greenLight = 1;
            redLight   = 0;
            motor = 1;
            buzzer = 1;
        }
        /**
         * @brief red on green off
        */
        void redLightOn(){
            redLight    = 1;
            greenLight  = 0;
            motor = 0;
            buzzer = 0;
            controlFlag = false;
        }
        /**
         * @brief check if member is in control
         * @return boolean flag of control value
        */
        bool isInControl(){
            return controlFlag;
        }
};
/// @brief initialise priority matrix
bool priority[2][3] = {0,0,0,
                       0,0,0};
///Initialise main traffic light                       
TrafficLight    tl1(0,priority,4,1,1,3,p20,p11,p23,p24);
///Initialise second traffic light
TrafficLight    tl2(1,priority,4,1,0,3,p19,p12,p25,p26);
///Initialise pedestrian light
PedLight        ped1(2,priority, 12,3,p27,p28,p21,p22,p18);
int main() {
    Serial pc(USBTX,USBRX);
    tl2.redLightOn();
    tl1.redLightOn();
    ped1.redLightOn();
    while(1){
        ///Create state change machine, pass control flags cyclically
        tl1.update_state(tl2.isInControl(), ped1.isInControl());
        tl2.update_state(tl1.isInControl(),ped1.isInControl());
        ped1.updateState(tl1.isInControl(), tl2.isInControl());
    }
}
