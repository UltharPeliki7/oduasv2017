#include "ros/ros.h"
#include "std_msgs/String.h"
#include "sensor_msgs/Range.h"
#include "std_msgs/Int16.h"
#include "std_msgs/Int64.h"
#include "std_msgs/Float64.h"
#include <iostream>
#include "math.h"
#include <cmath>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
//                                                  PLEASE CLEAN UP THE VARIABLE DECLARATIONS!!!

using namespace std; 								//Dock is W 10ft x  L 8ft

int US1 = 0;
int US2 = 0;
int US3 = 0;
int US4 = 0;
int LD1 = 0;
int LD2 = 0;

char tab2[1024];
char tab3[1024];

double Lat12 = 0;
double Lon12 = 0;
double Lat1 = 0;						//this is the current location Latitude from the GPS
double Lon1 = 0;						//this is the current location Longitude from the GPS
double Heading = 0;						//this is the current location Heading from the GPS
double FLat = 0;
double FLon = 0;
double BLat = 0;
double BLon = 0;
double Slat=0;
double Slon=0;
double R = 6371; //km "Earths Radius"				Need to check if this will work
double FH = 0;					//Final Heading from the bearing formula
double dist = 0; 				//Distance from the Distance formula

double NaviNodes[5000][2];//Nodes that were used for navigation
double Bnodes[600][2];//nodes for buoys

int distanceArr[190];
int currdist;
int currangle;
int lastdist;

int tbear = 0;
double lonx;
double laty;
int currObjCount = 0;   //current object/obstacle count

int idealspeed=0;
double setHeading = 0;

bool StartUp = true;
bool SpeedGate = false;
bool ObAvoid = false;
bool AutoDock = false;
bool EndRun = false;
bool frontisClear = true;

string LAT;
string LON;

/////////////////////////////////////OPENCV////////////////////////////////
string old_color = "red";
    double old_dist = 0;
    double old_angle = 0;
    string new_color;
    double new_dist;
    double new_angle;
/////////////////////////////////////////////////////////////////////////


std_msgs::String str_msg;


void UsCallback (const sensor_msgs::Range::ConstPtr& msg)
{
    US1 = msg->range;
}

void UsCallback1 (const sensor_msgs::Range::ConstPtr& msg1)
{
    US2 = msg1->range;
}

void UsCallback2 (const sensor_msgs::Range::ConstPtr& msg2)
{
    US3 = msg2->range;
}

void UsCallback3 (const sensor_msgs::Range::ConstPtr& msg3)
{
    US4 = msg3->range;
}

void LdCallBack (const sensor_msgs::Range::ConstPtr& msg4)
{
    LD1 = msg4->range;
    LD2 = msg4->field_of_view;
}

void GPSCallBack (const std_msgs::Int64::ConstPtr& msg5)
{
	LAT = msg5->data;

	strncpy(tab2,LAT.c_str(),sizeof(tab2));
	tab2[sizeof(tab2)-1]=0;

	Lat1=atof(tab2);
	Lat12=Lat1*pow(10,-7);
}
void GPSCallBack1 (const std_msgs::Int64::ConstPtr& msg6)
{

    	LON = msg6->data;

	strncpy(tab3,LON.c_str(),sizeof(tab3));
	tab3[sizeof(tab3)-1]=0;

	Lon1=atof(tab3);
	Lon12=Lon1*pow(10,-7);
}

void GPSCallBack2 (const std_msgs::Float64::ConstPtr& msg7)
{
    Heading = msg7->data;
}

double DegtoRad (double Deg)						//This converts degrees to Radians
{
    double radians = 0;
    double pi = atan(1)*4;

    radians = Deg * (pi / 180);

    return radians;
}

//LD1 is distance to an object
//LD2 is current lidar angle
//-----------------------------------------Lidar methods---------------------------------------------------


void BDtoDist(int bear, int dist, double Lit1, double Lin1, int bearb)   //Takes a bearing and a distance and using current position, finds the corresponding GPS point
{
	tbear=bear-90;
	tbear=tbear+bearb;
	if(tbear>=0 && tbear<=90)
	{
		lonx=Lin1+dist*cos(tbear);
		laty=Lit1+dist*sin(tbear);
	}

	if(tbear>90 && tbear<=180)
	{
		lonx=Lin1-dist*cos(tbear);
		laty=Lit1+dist*sin(tbear);
	}

	if(tbear>180 && tbear<270)
	{
		lonx=Lin1-dist*cos(tbear);
		laty=Lit1-dist*sin(tbear);
	}

	if(tbear>=270 && tbear<360)
	{
		lonx=Lin1+dist*cos(tbear);
		laty=Lit1-dist*sin(tbear);
	}


}

double findObstacle(int LD1, int LD2)
{
	currdist=LD1;
	currangle=LD2;
	if(currdist<(lastdist+10))
	{
		distanceArr[currangle]=currdist;
		BDtoDist(currangle,currdist,Lat1,Lon1,Heading);

 		cout << "Buoy detected!!!: " << currdist << "@"<< currangle<<"degrees"<< endl;

		Bnodes[currObjCount][0]=lonx;
		Bnodes[currObjCount][1]=laty;
		currObjCount++;
	}

 lastdist=currdist;
}


//------------------------------------------Navi Methods----------------------------------------------------
double Distance (double Lat1, double Lon1, double Lat2, double Lon2)	//Takes two coordinates and determines distance between those two points
{
    double a = 0;
    double c = 0;
    double R = 0;
    double dLat = 0;
    double dLon = 0;

    dLat = Lat2 - Lat1;
    dLon = Lon2 - Lon1;

    dLat = DegtoRad(dLat);
    dLon = DegtoRad(dLon);

    Lat1 = DegtoRad(Lat1);
    Lat2 = DegtoRad(Lat2);

    a = pow(sin(dLat / 2), 2) + cos(Lat1) * cos(Lat2) * pow(sin(dLon / 2), 2);

    c = 2 * atan2 (sqrt(a), sqrt(1 - a));

    R = 6371;    //km

    dist = R * c;

    dist = dist * 3280.8399;

    return dist;
}

double Buffer (double Lat1, double Lon1, double FH)						//This will give us the actual loc to try to reach with a buffer of 24inches from the given loc. Takes where we are, the bearing we need to reach the point we want to get too and the distance that we calculated we need to travel - the buffer distance to find the actual loc we want to try to reach.
{
	double D = 24; //inches
	double BH = 0;

	D = dist - D; 														//Take the distance from the distance formula minus the buffer dist

	BH = FH;


	Lat1 = DegtoRad(Lat1);
	Lon1 = DegtoRad(Lon1);

	BLat = asin(sin(Lat1) * cos(D/R) + cos(Lat1) * sin(D/R) * cos(BH));

	BLon = Lon1 + atan2(sin(BH) * sin(D/R) * cos(Lat1), cos(D/R) - sin(Lat1) * sin(BLat));

	return 0;
}


double Bearing (double Lat1, double Lon1, double Lat2, double Lon2)		//Takes two points and determines the bearing needed to reach the end point
{
    double dLon = 0;

    dLon = Lon2 - Lon1;

    double B1 = sin(dLon) * cos(Lat2);
    double B2 = cos(Lat1) * sin((sin(Lat2) - sin(Lat1))) * cos(Lat2) * cos(dLon);
    double Fhead = atan2 (B1, B2);

    FH = fmod(Fhead + 360, 360);
    FH = fmod(FH + 180, 360);

    return FH;
}

double Rotate (double angle, int Lmsg, int Rmsg)
{
	double Lspeed = (1500)+(400 * 40 * pow(10, -2));
	double Rspeed = (1500)+(400 * -20 * pow(10, -2));
	if ( Heading <= angle+2 || Heading >= angle-2 )
	{
		Lmsg = Lspeed;
		Rmsg = Rspeed;

	}
}

double SetHeading()
{
	setHeading=Heading;
	return setHeading;
}

double SetHeading(double x)
{
	setHeading=x;
	return setHeading;
}

int main(int argc, char **argv)
{
ros::init(argc, argv, "Navigation");		//Initializes the node Navigation
ros::NodeHandle nh;

//Sets up Subscriber to topic ultrasound sensors
ros::Subscriber sub1 = nh.subscribe("ultrasound1", 100, UsCallback);
ros::Subscriber sub2 = nh.subscribe("ultrasound2", 100, UsCallback1);
ros::Subscriber sub3 = nh.subscribe("ultrasound3", 100, UsCallback2);
ros::Subscriber sub4 = nh.subscribe("ultrasound4", 100, UsCallback3);

//Sets up Subscriber to topic Lidar
ros::Subscriber sub5 = nh.subscribe("Lidar", 100, LdCallBack);

//Sets up Subscriber to topic GPS
ros::Subscriber sub6 = nh.subscribe("Latitude", 100, GPSCallBack);
ros::Subscriber sub7 = nh.subscribe("Longitude", 100, GPSCallBack1);
ros::Subscriber sub8 = nh.subscribe("Heading", 100, GPSCallBack2);


//Setup the Publishers from the Navigation Node to the Thrusters
ros::Publisher Lnav_pub = nh.advertise<std_msgs::Int16>("Left", 100);
ros::Publisher Rnav_pub = nh.advertise<std_msgs::Int16>("Right", 100);



ros::Rate loop_rate(1);


while (ros::ok())
{
////////////////////////////////////////OPENCV//////////////////////////////////////////
        ifstream fread;
        fread.open("/home/oduasv/Desktop/Test1080/para.txt");
        fread>> new_color;
        fread >> new_dist;
        fread >> new_angle;
        // check if there exists updated paras
        if(new_color!=old_color || new_dist!=old_dist || new_angle!=old_angle){
             cout<<"color => "<<new_color<<endl;
             cout<<"dist : "<<new_dist<<endl;
             cout<<"angle : "<<new_angle<<endl;
             // update old paras
             old_color = new_color;
             old_dist = new_dist;
             old_angle = new_angle;

        }
        fread.close();
//////////////////////////////////////////////////////////////////////////////////////

	std_msgs::Int16 Lmsg;					//Sets up the messages to be read to send to the thrusters
    std_msgs::Int16 Rmsg;

 	if(StartUp==true)
	{
cout<<"We are starting up, setting left and right thrusters to 1500pwm"<<endl;
	//Slat=Lat12;
	//Slon=Lon12;
	StartUp=false;
	Lmsg.data=1500;
	Rmsg.data=1500;
	Lnav_pub.publish(Lmsg);					//Publish the data to the left thruster
    Rnav_pub.publish(Rmsg);
	}


Lmsg.data=1600;
Rmsg.data=1600;
Lnav_pub.publish(Lmsg);
Rnav_pub.publish(Rmsg);


/*
if((!new_color.compare("red"))&&new_angle>-2)
{cout<<"Entered red loop!!!"<<endl;
Lmsg.data=1500;
Rmsg.data=1590;
Lnav_pub.publish(Lmsg);					//Publish the data to the left thruster
    	Rnav_pub.publish(Rmsg);
}
else if((!new_color.compare("green"))&&new_angle<2)
{cout<<"Entered green loop!!!"<<endl;
Lmsg.data=1590;
Rmsg.data=1500;
Lnav_pub.publish(Lmsg);					//Publish the data to the left thruster
    	Rnav_pub.publish(Rmsg);
}
else {
cout<<"Going forward!!!"<<endl;
Lmsg.data=1520;
Rmsg.data=1520;
Lnav_pub.publish(Lmsg);					//Publish the data to the left thruster
    	Rnav_pub.publish(Rmsg);
}*/
//int speed =(1500)+(400 * 20*pow(10, -2));
cout<<Heading<<" Heading "<<endl;
cout << LD1<< " <LD1 "<<endl;
cout <<LD2<< " <LD2 " <<endl;
/*
if(LD1>1000&&LD2<90)
{Lmsg.data=1590;
Rmsg.data=1500;
Lnav_pub.publish(Lmsg);					//Publish the data to the left thruster
    	Rnav_pub.publish(Rmsg);
}
else if(LD1>1000&&LD2>90)
{Lmsg.data=1500;
Rmsg.data=1590;
Lnav_pub.publish(Lmsg);					//Publish the data to the left thruster
    	Rnav_pub.publish(Rmsg);
}
else if(LD1>1000&&LD2==90)
{Lmsg.data=1570;
Rmsg.data=1570;
Lnav_pub.publish(Lmsg);					//Publish the data to the left thruster
    	Rnav_pub.publish(Rmsg);
}
else
{Lmsg.data=1500;
Rmsg.data=1500;
Lnav_pub.publish(Lmsg);					//Publish the data to the left thruster
    	Rnav_pub.publish(Rmsg);
}
*/


/*
	if(Heading==setHeading)
	{
	Lmsg.data=speed;
	Rmsg.data=speed;
	SetHeading();
	}

	else
	   {
		if(Heading<setHeading)		// if the boat is yawing to the left, right motor cuts speed
		{
		Lmsg.data=speed;
		Rmsg.data=speed-100;
		}

		if(Heading>setHeading)		// if the boat is yawing to the right, left motor cuts speed
		{
		Lmsg.data=speed-100;
		Rmsg.data=speed;
		}
	   }
*/
//Prints GPS Values
cout<< "Lat: "<< Lat12 << endl;
cout<< "Lon: "<< Lon12 << endl;

    cout << "Left: " << Lmsg.data << endl;			//Prints the Left thruster value to the terminal
    cout << "Right: " << Rmsg.data << endl;			//Prints the Right thruster vlaue to the terminal
    cout << " "<< endl;


//*******ULTRA SONIC SENSORS*******
    cout << "ultrasonic 1: " << US1 << endl;			//Prints the values of the US Sensors to the Terminal
    cout << "ultrasonic 2: " << US2 << endl;
    cout << "ultrasonic 3: " << US3 << endl;
    cout << "ultrasonic 4: " << US4 << endl;

/*
//******LIDAR*******
*/
    cout << "lidar: " << LD1 << endl;				//Prints the Values of the Lidar to the Terminal
    cout << "lidar2: " << LD2 << endl;



    ros::spinOnce();
    loop_rate.sleep();
}

return 0;

}
