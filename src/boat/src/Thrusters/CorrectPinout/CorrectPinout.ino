
#include <Servo.h>
#include <ros.h>
#include <ros/time.h>
#include <std_msgs/String.h>
#include <std_msgs/Int16.h>
#include <std_msgs/Float64.h>
#define USE_USBCON



ros::NodeHandle nh;                          //Instantiate the Node Handle******

int LT;
int RT;


void LmessageCb(const std_msgs::Int16& msg)
{
  LT = msg.data;
}
void RmessageCb(const std_msgs::Int16& msg)
{
  RT = msg.data;
}

std_msgs::Int16 left;
std_msgs::Int16 right;

ros::Subscriber<std_msgs::Int16> L("Left", &LmessageCb);
ros::Subscriber<std_msgs::Int16> R("Right", &RmessageCb);

char var = 's';
//switched pins
byte leftmotorPin = 12;                      // Set Pin 9 to the output to the ESC (speed controller)
byte rightmotorPin = 13;                     // Set Pin 10 to the output to the ESC (speed controller)
int ch1 = 2;                                 // set the value coming from channel 1 of the receiver to Pin 2 (  Right Throttle  left(1000)/right(1750)/middle(1380)  )
int ch2 = 3;                                // set the value coming from channel 2 of the receiver to Pin 3 (  Right Throttle up(1000)/down(1750)/middle(1380)  )
int ch3 = 4;                               // set the value coming from channel 3 of the receiver to Pin 4 (  Left Throttle up(1000)/down(1750)/middle(1380)  )
int ch4 = 5;                               //set the value coming from channel 4 of the receiver to pin 5 (  Left Throttle left(1000)/right(1750)/middle(1380)  )
int ch5 = 6;                                //set the value coming from channel 5 of the receiver to pin 6 (it writes ch5 on the transmitter, also A, and Gear/Gyro)
int ch6 = 7;                                //set the value coming from channel 6 of the receiver to pin 7 (Ail/Elev/Rud Switch)
int spool = 11;                              //spooling servo pin
int userValue = 0; 

int LM = 0; 
int RM = 0; 
int LMtoESC = 1500; 
int RMtoESC = 1500;
int idle = 1500;

int lastLMspeed = 0;                     // Initialize the last speed of the Left motor as 1500
int lastRMspeed = 0;                     // Initialize the last speed of the Right motor as 1500
int LMspeed = 1500;
int RMspeed = 1500; 



void setup() 
{
        nh.initNode();                           //Initialize the Node Handle*****
        nh.subscribe(L);                       //Advertise any topic being published*******
        nh.subscribe(R);                       //Advertise any topic being published*******
        
        Serial.begin(57600);                    // Begin Serial communication with the computer
        pinMode(ch1, INPUT);                     // Set Pin for ch2 as input  
        pinMode(ch2, INPUT);                     // Set Pin for ch3 as input
        pinMode(ch3, INPUT);                   
        pinMode(ch4, INPUT);
        pinMode(ch5, INPUT);                    // Set Pin for ModePin as an Input
        pinMode(ch6, INPUT);
	
       // leftmotor.attach(leftmotorPin);          // Attach Left motor output to Pin 9
      //  rightmotor.attach(rightmotorPin);        // Attach Right motor output to Pin 7

	//leftmotor.writeMicroseconds(1500);       // send "stop" signal to ESC.
       // rightmotor.writeMicroseconds(1500);      // send "stop" signal to ESC.
	delay(500);                             // delay to allow the ESC to recognize the stopped signal and initialize
       LM = map(pulseIn(3, LOW), 17500, 18800, 1100, 1900);          // Read the value from the left joystick of the remote control
        RM = map(pulseIn(5, LOW), 17500, 18800, 1100, 1900);         // Read the value from the right joystick of the remote control
}

void loop()
{Serial.print("ch1: ");
  Serial.println(pulseIn(ch1, HIGH));
  Serial.print("ch2: ");
  Serial.println(pulseIn(ch2, HIGH));
  Serial.print("ch3: ");
  Serial.println(pulseIn(ch3, HIGH));
  Serial.print("ch4: ");
  Serial.println(pulseIn(ch4, HIGH));
  Serial.print("ch5: ");
  Serial.println(pulseIn(ch5, HIGH));
  Serial.print("ch6: ");
  Serial.println(pulseIn(ch6, HIGH));
  Serial.println();
  
  
 delay(1000);
 
 
}

