#include "OVR_CAPI.h"
#include "Kernel\OVR_Math.h"
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <phidget21.h>


using namespace OVR;
using namespace std;

ovrHmd hmd;
ovrFrameTiming frameTiming;

char printbuf[100];

///-------------------------- Phidget Servo Handlers --------------------------
int CCONV AttachHandler(CPhidgetHandle SERV, void *userptr)
{
        int serialNo;
        const char *name;

        CPhidget_getDeviceName (SERV, &name);
        CPhidget_getSerialNumber(SERV, &serialNo);
        sprintf_s(printbuf, "%s %10d attached!", name, serialNo);
        std::cout << printbuf << endl;
        return 0;
}

int CCONV DetachHandler(CPhidgetHandle SERV, void *userptr)
{
        int serialNo;
        const char *name;

        CPhidget_getDeviceName (SERV, &name);
        CPhidget_getSerialNumber(SERV, &serialNo);
        sprintf_s(printbuf, "%s %10d detached!", name, serialNo);
        cout << printbuf << endl;
        return 0;
}

int CCONV ErrorHandler(CPhidgetHandle SERV, void *userptr, int ErrorCode, const char *Description)
{
        sprintf_s(printbuf, "Error handled. %d - %s", ErrorCode, Description);
        cout << printbuf << endl;
        return 0;
}

int CCONV PositionChangeHandler(CPhidgetServoHandle SERV, void *usrptr, int Index, double Value)
{
        //sprintf_s(printbuf, "Motor: %d > Current Position: %f", Index, Value);
        //cout << printbuf << endl;
        return 0;
}

//Display the properties of the attached phidget to the screen.  We will be displaying the name, serial number and version of the attached device.
int display_properties(CPhidgetServoHandle phid)
{
        int serialNo, version, numMotors;
        const char* ptr;

        CPhidget_getDeviceType((CPhidgetHandle)phid, &ptr);
        CPhidget_getSerialNumber((CPhidgetHandle)phid, &serialNo);
        CPhidget_getDeviceVersion((CPhidgetHandle)phid, &version);

        CPhidgetServo_getMotorCount (phid, &numMotors);

        sprintf_s(printbuf, "%s\n", ptr);
        cout << printbuf << endl;

        sprintf_s(printbuf, "Serial Number: %10d\nVersion: %8d\n# Motors: %d\n", serialNo, version, numMotors);
        cout << printbuf << endl;

        return 0;
}
///-------------------------- Phidget Servo Handlers --------------------------

//saturate the x value to a< x <b
float saturate(float x, float a, float b){
        if(x < a)
                return a;
        else if (x > b)
                return b;
        else
                return x;
}



//-------------------------
//Initialize Oculus Rift
void Init()
{
        ovr_Initialize();
        hmd = ovrHmd_Create(0);

        if(!hmd) return;
        ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_Position, 0);
}

//Clear Oculus
void Clear()
{
        ovrHmd_Destroy(hmd);
        ovr_Shutdown();
}

void Output()
{
//      HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
//      CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
//      GetConsoleScreenBufferInfo(h, &bufferInfo);

        ///-------------------------Phidget Servo Code-------------------------
        //Taken from phidget servo-simple C++ code
        int result;
        //double curr_pos;
        const char *err;

        //offsets for the servo position
        int offsetYaw = 120;
        int offsetPitch = 140;


        //Declare an servo handle
        CPhidgetServoHandle servo = 0;

        //create the servo object
        CPhidgetServo_create(&servo);

        //Set the handlers to be run when the device is plugged in or opened from software, unplugged or closed from software, or generates an error.
        CPhidget_set_OnAttach_Handler((CPhidgetHandle)servo, AttachHandler, NULL);
        CPhidget_set_OnDetach_Handler((CPhidgetHandle)servo, DetachHandler, NULL);
        CPhidget_set_OnError_Handler((CPhidgetHandle)servo, ErrorHandler, NULL);

        //Registers a callback that will run when the motor position is changed.
        //Requires the handle for the Phidget, the function that will be called, and an arbitrary pointer that will be supplied to the callback function (may be NULL).
        CPhidgetServo_set_OnPositionChange_Handler(servo, PositionChangeHandler, NULL);

        //open the servo for device connections
        CPhidget_open((CPhidgetHandle)servo, -1);

        //get the program to wait for an servo device to be attached
        cout << "Waiting for Servo controller to be attached...." << endl;
        if((result = CPhidget_waitForAttachment((CPhidgetHandle)servo, 10000)))
        {
                CPhidget_getErrorDescription(result, &err);
                sprintf_s(printbuf, "Problem waiting for attachment: %s\n", err);
                cout << printbuf << endl;
                return;
        }

        //change the motor position
        //valid range is -22 to 232
        //we'll set it to a few random positions to move it around

        CPhidgetServo_setPosition (servo, 0, offsetYaw);
        CPhidgetServo_setPosition (servo, 1, offsetPitch);
        ///-------------------------Phidget  Servo Code-------------------------
        cout << endl;
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
        GetConsoleScreenBufferInfo(h, &bufferInfo);


        bool count = true;              //added
        float initYaw, servoYaw;
        ///++++++++++++++++++++++++-Update Code-++++++++++++++++++++++++
        while(hmd)
        {
                frameTiming = ovrHmd_BeginFrameTiming(hmd, 0);
                ovrTrackingState ts = ovrHmd_GetTrackingState(hmd, frameTiming.ScanoutMidpointSeconds);

                if(ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
                {
                        Posef pose = ts.HeadPose.ThePose;
                        float yaw, pitch, roll;
                        //convert raw pos values to YPR pos values
                        pose.Rotation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
                        if (count)
                        {
                                pose.Rotation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&initYaw, &pitch, &roll);
                                initYaw = RadToDegree(initYaw);
                                count = !count;
                                cout << "initYaw: " << RadToDegree(initYaw) << endl;
                                cout << "offsetYaw: " << offsetYaw << endl << endl;

                        }
                        SetConsoleCursorPosition(h, bufferInfo.dwCursorPosition);

                        pose.Rotation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);

                        cout << "yaw: " << RadToDegree(yaw) << endl;
                        cout << "pitch: " << RadToDegree(pitch) << endl;
                        cout << "roll: " << RadToDegree(roll) << endl;

                        if (initYaw <= 0 ) {
                                if (RadToDegree(yaw) <= 0) servoYaw = saturate(abs((RadToDegree(yaw)))+22, 22, 212);
                        } else {
                                if (RadToDegree(yaw) > 0) servoYaw = saturate(abs((RadToDegree(yaw))-180)+22, 22, 212);
                        }

                        //write pos to servos

                        CPhidgetServo_setPosition (servo, 0, servoYaw);
                        CPhidgetServo_setPosition (servo, 1, saturate((RadToDegree(pitch)+offsetPitch), 11, 175));


                        if (_kbhit())
                                exit(0);
                }
                Sleep(50);
                ovrHmd_EndFrameTiming(hmd);
        }
        ///++++++++++++++++++++++++-Oculus Rift Update Code-++++++++++++++++++++++++

        //Disengage servos
        CPhidgetServo_setEngaged (servo, 0, 0);
        CPhidgetServo_setEngaged (servo, 1, 0);

        //this is a signal to terminate the program so we will close the phidget and delete the object we created
        cout << "Closing..." << endl;
        CPhidget_close((CPhidgetHandle)servo);
        CPhidget_delete((CPhidgetHandle)servo);
}

int main()
{
        Init();
        Output();
        Clear();
        return 0;
}