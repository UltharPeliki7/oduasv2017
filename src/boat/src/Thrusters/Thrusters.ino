
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
Servo leftmotor;                            // declare the left motor output variable (ESC)
Servo rightmotor;                           // declare the right motor output variable (ESC)

void setup() 
{
        nh.initNode();                           //Initialize the Node Handle*****
        nh.subscribe(L);                       //Advertise any topic being published*******
        nh.subscribe(R);                       //Advertise any topic being published*******
        
        Serial.begin(57600);                    // Begin Serial communication with the computer
        pinMode(ch2, INPUT);                     // Set Pin for ch2 as input  
        pinMode(ch3, INPUT);                     // Set Pin for ch3 as input
        pinMode(ch5, INPUT);                   // Set Pin for ModePin as an Input  
        pinMode(ch6, INPUT);                    //Set pin for ail/elev/rud as an input, allowing us to swap forward thrust for reverse thrus
	//pinMode(ch, INPUT);

       // pinMode(MotorLeft, OUTPUT);
       // pinMode(MotorRight, OUTPUT);
        //pinMode(MotorTop, OUTPUT); 
      //  pinMode(pol,OUTPUT);
       // digitalWrite(pol,LOW);
      //  pinMode(spool,OUTPUT);
      //  spooler.attach(spool);
      //  digitalWrite(spool,LOW);
        leftmotor.attach(leftmotorPin);          // Attach Left motor output to Pin 9
        rightmotor.attach(rightmotorPin);        // Attach Right motor output to Pin 7

	leftmotor.writeMicroseconds(1500);       // send "stop" signal to ESC.
        rightmotor.writeMicroseconds(1500);      // send "stop" signal to ESC.
	delay(500);                             // delay to allow the ESC to recognize the stopped signal and initialize
       LM = map(pulseIn(ch2, LOW), 17500, 18800, 1100, 1900);          // Read the value from the left joystick of the remote control
        RM = map(pulseIn(ch3, LOW), 17500, 18800, 1100, 1900);         // Read the value from the right joystick of the remote control
}


/*
/************ First step in the loop is to determine what mode the vessel is operating in according to the button that is on the vessel********
*/

void loop()
{
  
  
  //As map implies, it maps the first value range to the second value range. EX: map(input, 100,200,0,10)  would map a range of numbers 100-200 to 0-10. 110 would map to 1. 
  //mapping map(200,100,0,10) would reverse the mapping. 110 would map to 9, 120 would map to 8, etc.
 /* Serial.println(map(pulseIn(ch,HIGH),1400,2600,0,20));
  
if((map(pulseIn(ch,HIGH),1400,2600,0,20)>=6) && (map(pulseIn(ch,HIGH),1400,2600,0,10) <=14))
{
  spooler.write(90);
  
}
else if((map(pulseIn(ch,HIGH),1400,2600,0,10)<6))
{
  spooler.write(0);
  
}
else if((map(pulseIn(ch,HIGH),1400,2600,0,10)>14))
{
  spooler.write(300);
  
}
*/
  userValue = map(pulseIn(ch5, HIGH),1200,1800,10,-10);                  // Read input value  of the Mode switch
   
  if (userValue > 0) {      /// If mode switch is > 0 
  
  left.data = LT;
  right.data = RT;
  
  leftmotor.writeMicroseconds(LT);
  rightmotor.writeMicroseconds(RT);
  
   Serial.println("Autonomous Mode");   // Print Autonomous mode so we know what mode it is in. Should be for debugging purposes ONLY
   Serial.println(" ");
   Serial.println(LT);
   Serial.println(RT);
   Serial.println(" ");
   
    nh.spinOnce();
   
    
   }
/*
/ **********This begins the point of the code for using the remote control to control the vessel************
*/

    else //if(pulseIn(ch1,HIGH)<2000) //checks to see if the ail/eve/rud switch is flipped in one direction or the other
    {         //If mode switch is 0 means the vessel is in remote control mode
if(pulseIn(ch6, HIGH)<1200)
{
        LM = map(pulseIn(ch2, HIGH), 1750, 1000, 1100, 1900);          // Read the value from the left joystick of the remote control
        RM = map(pulseIn(ch3, HIGH), 1750, 1000, 1100, 1900);         // Read the value from the right joystick of the remote control
}
else
{       LM = map(pulseIn(ch2, HIGH), 1750, 1000, 1900, 1100);          // Read the value from the left joystick of the remote control
        RM = map(pulseIn(ch3, HIGH), 1750, 1000, 1900, 1100);         // Read the value from the right joystick of the remote control
}
  
        
 Serial.println(LM);
 Serial.println(RM);
      LMtoESC=LM;
      RMtoESC=RM;
        
       if(LMtoESC >= 1460 && LMtoESC <= 1540 && RMtoESC >= 1460 && RMtoESC <= 1540)                  // Checks the value of the Left speed and determines if it is within the idle range
  {
    leftmotor.writeMicroseconds(idle);                    // Send signal to ESC for left motor
    rightmotor.writeMicroseconds(idle);
        
  }
  
  else if((LMtoESC >= 1460 && LMtoESC <= 1540) && (RMtoESC < 1460 || RMtoESC > 1540))                  // Checks the value of the Left speed and determines if it is within the idle range
  {
    leftmotor.writeMicroseconds(idle);                   // Send signal to ESC for left motor
    rightmotor.writeMicroseconds(RMtoESC);
   
    
  }
  
  else if((RMtoESC >= 1460 && RMtoESC <= 1540) && (LMtoESC < 1460 || LMtoESC > 1540))                  // Checks the value of the Left speed and determines if it is within the idle range
  {
    rightmotor.writeMicroseconds(idle);                   // Send signal to ESC for left motor
    leftmotor.writeMicroseconds(LMtoESC);
    
  }
  
  else
  {
     leftmotor.writeMicroseconds(LMtoESC);
     rightmotor.writeMicroseconds(RMtoESC);
   
  }
  
  Serial.println(LMtoESC); //For debugging purposes ONLY
  Serial.println(RMtoESC);
  Serial.println(" ");

                        
 }
 /*else //ROV is to be controlled (Not used currently)
 {
   Serial.println("RoV control enabled."); 
        LM = map(pulseIn(3, HIGH), 2500, 1400, 1100, 1900);          // Read the value from the left joystick of the remote control
        RM = map(pulseIn(2, HIGH), 2500, 1400, 1100, 1900);         // Read the value from the right joystick of the remote control
   
    if(LM >= 1460 && LM <= 1540 && RM >= 1460 && RM <= 1540)                  // Checks the value of the Left speed and determines if it is within the idle range
  {
    Forward(500);
  }
   
  else if((LM>= 1460 && LM <= 1540) && (RM < 1460 || RM > 1540))                  // Checks the value of the Left speed and determines if it is within the idle range
  {
  
    Right(100);
   
    
  }
  
  else if((RM>= 1460 && RM <= 1540) && (LM < 1460 || LM > 1540))                  // Checks the value of the Left speed and determines if it is within the idle range
  {
    Left(100);
  }
  
  else if((RM > 1540) && (LM > 1540))                  // Checks the value of the Left speed and determines if it is within the idle range
  {
    Down(100);
  }
  else if((RM < 1480) && (LM < 1480))                  // Checks the value of the Left speed and determines if it is within the idle range
  {
    Up(100);
  }
  
  Serial.println(LM);
  Serial.println(RM);
  Serial.println(" ");

                        
     delay(100);
     
   
   
   
   
 }*/
 
 delay(100);
}

