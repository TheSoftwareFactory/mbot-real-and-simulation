#include "ros/ros.h"
#include "std_msgs/String.h"
#include <image_transport/image_transport.h>
#include "sensor_msgs/Range.h"
#include "sensor_msgs/Illuminance.h"
#include <geometry_msgs/Twist.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#define FORWARD 0
#define TWIST 1
#define FOLLOW 2
#define INTERCEIPT 3
#define EXPLORE 4
#define SPEED 0.1
#define RESET 1
#define D 0.00998 //stip width
#define LFOV 0.0129

#define INTERC_ROT_SPEED 0.2

float illuR = 178, illuL = 178, range = 1, Delta = 0, Sigma = 356, lum1 = 178, lum2 = 0,  lr = 0, ll = 0, alpha = 0; //init to try to fix the startup
int interc_r = 0, interc_l = 0, interc_f = 0, twist_f = 0, randomrl = 0;

// Create Twist message
geometry_msgs::Twist twist;

void driver(int order, int reset_loop_c = 0)
{
	static int loopc = 0;

	float rotation = 0, speed = 0;

	if (reset_loop_c)
		loopc = 0;
	switch (order)
	{

	case FORWARD: //If there are no obstacles in front go forward

		//ROS_INFO("IN FORWARD");
		twist.linear.x = SPEED;
		twist.linear.y = 0;
		twist.linear.z = 0;

		twist.angular.x = 0;
		twist.angular.y = 0;
		twist.angular.z = 0;
		break;

	case TWIST: //If there are obstacles in front stop and turn for 2 seconds on one side

		ROS_INFO("IN TWIST");
		twist.linear.x = 0;
		twist.linear.y = 0;
		twist.linear.z = 0;

		twist.angular.x = 0;
		twist.angular.y = 0;
		twist.angular.z = 1;
		if (loopc++ > 200)
		{
			twist_f = 0;
			loopc = 0;
		}
		break;

	case FOLLOW:

		//Delta = (illuL - illuR);
		if (Delta > 0)
			rotation = /*-(0.05)*(1-exp(abs(Delta/10))) ;	*/ -0.0005 * abs(Delta); //(0.0001)*exp(abs(Delta/20));
		else
			rotation = /*(0.05)*(1-exp(abs(Delta/10))); */ 0.0005 * abs(Delta); //-(0.0001)*exp(abs(Delta/20));

		if (rotation > 0.3)
			rotation = 0.3;
		else if (rotation < -0.3)
			rotation = -0.3;

		ROS_INFO("IN FOLLOW, L[%f] R[%f] D[%f] S[%f] Rot[%f]", illuL, illuR, Delta, Sigma, rotation);

		twist.linear.x = SPEED; //0.05 m/s
		twist.linear.y = 0;
		twist.linear.z = 0;

		twist.angular.x = 0;
		twist.angular.y = 0;
		twist.angular.z = rotation;
		break;

	case INTERCEIPT: // il faut sortir du mode interception quand on est dans l'axe depuis 1 seconde (speed n'est pas passer a 0

		if (interc_l) //interception de ligne virage a gauche
		{
			if (illuR > 109 && Sigma>220) //capteur ext sort de la ligne 
			{
				rotation = INTERC_ROT_SPEED;
				speed = 0.005;
				loopc = 0;
			}
			else
			{
				if (Sigma<220 && abs(Delta)<90) //Proche de l'axe
				{
					if (randomrl < 50) rotation = -0.0005 * Delta;
					else rotation = 0.0005 * Delta;
					if (loopc++ > 200)
					{
						interc_l = 0;
						interc_f = 0;
					}
				}
				else
				{
					rotation = -0.0005 * Delta;
					//rotation = -0.003;
				}
				speed = 0.015;
			}
		ROS_INFO("IN INTERCEIPT_L L[%f] R[%f] S[%f] D[%f] ROT[%f]", illuL, illuR, Sigma, Delta, rotation);
		}
		if (interc_r) //interception de ligne virage a droite
		{
			if (illuL > 109 && Sigma>220)
			{
				rotation = -INTERC_ROT_SPEED;
				speed = 0.005;
				loopc = 0;
			}
			else
			{
				if (Sigma<220 && abs(Delta)<90) //Proche de l'axe
				{
					rotation = -0.0005 * Delta;
					if (loopc++ > 200)
					{
						interc_r = 0;
						interc_f = 0;
					}
				}
				else
				{
					rotation = -0.0005 * Delta;
					//rotation = 0.003;
				}
				speed = 0.015;
			}
		ROS_INFO("IN INTERCEIPT_R L[%f] R[%f] S[%f] D[%f] ROT[%f]", illuL, illuR, Sigma, Delta, rotation);
		}

		twist.linear.x = speed;
		twist.linear.y = 0;
		twist.linear.z = 0;

		twist.angular.x = 0;
		twist.angular.y = 0;
		twist.angular.z = rotation;
		break;
	}
	return;
}

//Some kind of god of war exploration

void explorer()
{
	if ((abs(Delta) > 125) || (illuR < 80) || (illuL < 80) || interc_l || interc_r)
	{
		if (illuR < 10 && illuL < 10)
			interc_f = 1;
		if (!interc_l && !interc_r)
		{
			if (Delta > 0)
			{
				interc_l = 1;
				interc_r = 0;
			}
			else
			{
				interc_l = 0;
				interc_r = 1;
			}
			driver(INTERCEIPT, RESET);
		}
		else
		{
			driver(INTERCEIPT);
		}
	}

	if ((illuR > 90) && (illuR < 140) && (illuL > 90) && (illuL < 140) && !interc_l && !interc_r)
	{
		driver(FOLLOW);
	}

	if (illuR > 170 && illuL > 170 && !interc_f) //Threshold to be defined
	{
		interc_r = 0;
		interc_l = 0;
		if (range < 0.6 || twist_f) //Threshold to be defined
		{
			if(!twist_f)
			{
				twist_f = 1;
				driver(TWIST, RESET); //robot turns 45° and stops
				randomrl = rand();
			} else
				driver(TWIST); //robot turns 45° and stops
		}
		else
			driver(FORWARD); //robot goes forward
	}

	return;
}

void rangeCallback(const sensor_msgs::Range::ConstPtr &msg)
{
	//ROS_INFO("Sonar Seq: [%d]", msg->header.seq);//Debug message console
	//ROS_INFO("Sonar Range: [%f]", msg->range);//Debug message console
	range = msg->range;
}

void illurCallback(const sensor_msgs::Illuminance::ConstPtr &msg)
{
	// ROS_INFO("Illuminance Right Seq: [%d]", msg->header.seq);//Debug message console
	//ROS_INFO("Illuminance Right Value: [%f]", msg->illuminance);//Debug message console
	illuR = msg->illuminance;
	if (illuR<lum1 && illuL>lum2)
	{
		lr = LFOV*(illuR-lum1)/(lum2-lum1);
		//ROS_INFO("lr[%fmm]", lr*1000);
	}
	else
		lr = -1;
}

void illulCallback(const sensor_msgs::Illuminance::ConstPtr &msg)
{
	// ROS_INFO("Illuminance Left Seq: [%d]", msg->header.seq);//Debug message console
	//ROS_INFO("Illuminance Left Value: [%f]", msg->illuminance);//Debug message console
	illuL = msg->illuminance;

	if (illuL<lum1 && illuL>lum2)
	{
		ll = LFOV*(illuL-lum1)/(lum2-lum1);
		//ROS_INFO("lg[%fmm]", ll*1000);
		//ROS_INFO("width[%fmm]", (ll+lr)*1000);
	}else
		ll = -1;

	if (lr>0 && ll>0)
	{
		//ROS_INFO("lr[%fmm]", lr*1000);
		//ROS_INFO("lg[%fmm]", ll*1000);
		ROS_INFO("wi[%fmm]", (ll+lr)*1000);
		//ROS_INFO("acos[%f]", (D/(ll+lr)));
		alpha = acos(D/(ll+lr));
		ROS_INFO("alpha[%f]", alpha/M_PI*180);
	}else
		alpha = -1;
	
	Delta = illuL - illuR;
	Sigma = illuL + illuR;
}

void camrCallback(const sensor_msgs::Image::ConstPtr &msg)
{
	//ROS_INFO("Checking on right IR sensor");//Debug message console
}

void camlCallback(const sensor_msgs::Image::ConstPtr &msg)
{
	//ROS_INFO("Checking on left IR sensor");//Debug message console
}

//////////////////////////////////////////////////////////////////////////////////////
//Main function

int main(int argc, char **argv)
{
	/**
   * The ros::init() function needs to see argc and argv so that it can perform
   * any ROS arguments and name remapping that were provided at the command line.
   * For programmatic remappings you can use a different version of init() which takes
   * remappings directly, but for most command-line programs, passing argc and argv is
   * the easiest way to do it.  The third argument to init() is the name of the node.
   *
   * You must call one of the versions of ros::init() before using any other
   * part of the ROS system.
   */
	ros::init(argc, argv, "general_listener"); //Node's name is general listener

	/**
   * NodeHandle is the main access point to communications with the ROS system.
   * The first NodeHandle constructed will fully initialize this node, and the last
   * NodeHandle destructed will close down the node.
   */
	ros::NodeHandle n; //Node's handler name is n
					   /**
   * The subscribe() call is how you tell ROS that you want to receive messages
   * on a given topic.  This invokes a call to the ROS
   * master node, which keeps a registry of who is publishing and who
   * is subscribing.  Messages are passed to a callback function, here
   * called chatterCallback.  subscribe() returns a Subscriber object that you
   * must hold on to until you want to unsubscribe.  When all copies of the Subscriber
   * object go out of scope, this callback will automatically be unsubscribed from

   * this topic.
   *
   * The second parameter to the subscribe() function is the size of the message
   * queue.  If messages are arriving faster than they are being processed, this
   * is the number of messages that will be buffered up before beginning to throw
   * away the oldest ones.
   */

	//ROS_INFO("IN DRIVER");
	ros::NodeHandle nh; //Node's handler name is nh

	// Init cmd_vel publisher
	ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);

	//Subscribe to this topic, ????, Use this fonction when something is published on this topic

	ros::Subscriber subson = n.subscribe("/sensor/ir_front", 1000, rangeCallback);
	ros::Subscriber subIRR = n.subscribe("/light_sensor_pluginD/lightSensorD", 1000, illurCallback);
	ros::Subscriber subIRL = n.subscribe("/light_sensor_pluginL/lightSensorL", 1000, illulCallback);
	ros::Subscriber subcamR = n.subscribe("/cameraR/rgb/image_raw", 1000, camrCallback);
	ros::Subscriber subcamL = n.subscribe("/cameraL/rgb/image_raw", 1000, camlCallback);

	/**
   * ros::spin() will enter a loop, pumping callbacks.  With this version, all
   * callbacks will be called from within this thread (the main one).  ros::spin()
   * will exit when Ctrl-C is pressed, or the node is shutdown by the master.

   1 ros::Rate r(10); // 10 hz
   2 while (should_continue)
   3 {90
   4   ... do some work, publish some messages, etc. ...
   5   ros::spinOnce();
   6   r.sleep();
   7 }
   */
	for (int i = 0; i < 100; i++)
	{
		usleep(10000);
		ros::spinOnce();
	}
	ros::Rate r(100);
	while (ros::ok())
	{
		explorer();
		pub.publish(twist); //send command to robot actuators
		ros::spinOnce();
		r.sleep();
	}
	return 0;
}