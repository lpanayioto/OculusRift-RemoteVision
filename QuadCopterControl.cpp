#include "mbed.h"
#include "rtos.h"


// Serial
Serial pc(USBTX,USBRX);

// delay between signals in microseconds (µSec)
double pulseInterval = 380;
int channels = 7;

// uptime of each signal in microseconds
double Throttle_Uptime = 1080;
double Roll_Uptime = 1080;
double Pitch_Uptime = 1080;
double Yaw_Uptime = 1080;
double Enable_Uptime = 1500;
double Stabilize_Uptime = 1500;
double Extra_Uptime = 0;		//Extra Channel
bool manualLock = true;

//Threading Variables
Mutex pwm_mutex;
Semaphore var_change(2);

// setup file read location
LocalFileSystem local("local");

// vars for file reading
int Throttle = 0;
int Roll = 0;
int Pitch = 0;
int Yaw = 0; 
int Enable = 0;
int Stability = 0;
//Trash var for file reading
char trash[16];

// uptime dependant on other signals
double Remainder_Uptime = 22000.0 - ((pulseInterval * channels) + Throttle_Uptime + Roll_Uptime + Pitch_Uptime + Yaw_Uptime + Enable_Uptime + Stabilize_Uptime + Extra_Uptime);
// set "soft pwm" output pin
DigitalOut PWMPIN(p21);


//Change values
void changeDelayValues(int newThrottle, int newRoll, int newPitch, int newYaw, int newEnable, int newStability)
{
    //var_change.wait();
    Throttle_Uptime = newThrottle;
    Roll_Uptime = newRoll;
    Pitch_Uptime = newPitch;
    Yaw_Uptime = newYaw;
    Enable_Uptime = newEnable;
    Stabilize_Uptime = newStability;
    Remainder_Uptime = 22000.0 - (pulseInterval * channels + Throttle_Uptime + Roll_Uptime + Pitch_Uptime + Yaw_Uptime + Enable_Uptime + Stabilize_Uptime + Extra_Uptime);
    //var_change.release();
}

// Reads the file on the Mbed called "setup.txt"
void ReadFile (void)
{
    FILE *set = fopen("/local/setup.txt", "r"); // Open "setup.txt" on the local file system for read
    fscanf(set,"%s %d",trash,&Throttle);
    fscanf(set,"%s %d",trash,&Roll);
    fscanf(set,"%s %d",trash,&Pitch);
    fscanf(set,"%s %d",trash,&Yaw);
    fscanf(set,"%s %d",trash,&Enable);
    fscanf(set,"%s %d",trash,&Stability);
    fclose(set);
}
/// +++++++++++++++++++++++++++ New Code +++++++++++++++++++++++++++ ///
DigitalOut throttleLed(p29);
DigitalOut pitchLed(p27);
DigitalOut rollLed(p25);
DigitalOut yawLed(p23);

int minWidth = 700;		//8µSec == 1%
int maxWidth = 1500;
int defWidth = 1100;
//int defWidth = 1080;

// LED Check --DEBUG--
void commandFeedback() {
    if (Throttle > minWidth) throttleLed = 1;
    else throttleLed = 0;
    
    if (Pitch != defWidth) pitchLed = 1;
    else pitchLed = 0;
    
    if (Roll != defWidth) rollLed = 1;
    else rollLed = 0;
    
    if (Yaw != defWidth) yawLed = 1;
    else yawLed = 0;
}

// Dynamic inputs via serial
void getSerial(void){
	char buff[5];
	int i=0;
	
/*	while(i<5){
		pc.printf("Enter Char: ");
		buff[i] = pc.getc();	//get char from serial
		pc.putc(buff[i]);		//echo char on serial
		
		if((buff[i] == "t") || (buff[i] == "p") || (buff[i] == "r") || (buff[i] == "y") || (buff[i] == "e") || (buff[i] == "s"))
		{
			for(int j=0;j<4;j++)
			{
				buff[j] = pc.getc();	//get char from serial
				pc.putc(buff[i]);		//echo char on serial
			}
		}
		else
			pc.printf("Unknown -- Use t,p,r,y,e,s\n");

	}
*/	
	
}

int throttleCeiling = 1100;
int throttleInc = 20;
bool increaseThrottle = true;

// Thread for input changes
void commVarsThread(void const *args)
{
    // listen
    while(true) {
        while(!manualLock) {
            ReadFile();
            /*
Throttle = defWidth;
Roll = defWidth;
Pitch = defWidth;
Yaw = defWidth;
Enable = 1500;
Stability = 1500;
*/
            /*
if (increaseThrottle) {
if (Throttle + throttleInc > throttleCeiling) {
Throttle = throttleCeiling;
increaseThrottle = false;
}
else Throttle += throttleInc;
}
else {
Throttle = 700;
}
*/
			//getSerial();
            changeDelayValues(Throttle, Roll, Pitch, Yaw, Enable, Stability);
            Thread::wait(300);
        }

    }
}

// function to generate pwm
void generatePWMSignal(double bigDelayWidth, double throttleWidth, double RollWidth, double PitchWidth, double YawWidth, double enableWidth, double stabilizeWidth, double pulseIntervalDelay)
{
    manualLock = true;
    // long remainder uptime
    PWMPIN = 1; //Soft pwm

	//???
    manualLock = false;
    wait_us(10000);
    //var_change.wait();
    manualLock = true;
    wait_us(bigDelayWidth - 10000);
    PWMPIN = 0;
    //SYNCPIN = 0;
    wait_us(pulseIntervalDelay);
	
    // throttle pulse
    PWMPIN = 1;
    wait_us(throttleWidth);
    PWMPIN = 0;
    wait_us(pulseIntervalDelay);
	
    // Roll pulse
    PWMPIN = 1;
    wait_us(RollWidth);
    PWMPIN = 0;
    wait_us(pulseIntervalDelay);

    // Pitch pulse
    PWMPIN = 1;
    wait_us(PitchWidth);
    PWMPIN = 0;
    wait_us(pulseIntervalDelay);
	
    // Yaw pulse
    PWMPIN = 1;
    wait_us(YawWidth);
    PWMPIN = 0;
    wait_us(pulseIntervalDelay);
	
    // Enable pulse
    PWMPIN = 1;
    wait_us(enableWidth);
    PWMPIN = 0;
    wait_us(pulseIntervalDelay);
	
    // Stability pulse
    PWMPIN = 1;
    wait_us(stabilizeWidth);
    PWMPIN = 0;
    wait_us(pulseIntervalDelay);
    //var_change.release();
    
	// LED Check --DEBUG--
    commandFeedback();
}


// separate thread for data stream and variable change for PWM
void PWMgen(void const *args)
{
	generatePWMSignal(Remainder_Uptime, Throttle_Uptime, Roll_Uptime, Pitch_Uptime, Yaw_Uptime, Enable_Uptime, Stabilize_Uptime, pulseInterval);
}

int main()
{

    /// generate pwm signal \\\
    // changeDelayValues();
    Thread thread(commVarsThread);
	//Thread thread(PWMgen);
    while(true) {
        //data_delay.wait();
        generatePWMSignal(Remainder_Uptime, Throttle_Uptime, Roll_Uptime, Pitch_Uptime, Yaw_Uptime, Enable_Uptime, Stabilize_Uptime, pulseInterval);
        //data_delay.release();
    }
}