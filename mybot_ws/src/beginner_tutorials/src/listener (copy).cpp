#include "ros/ros.h"
#include "std_msgs/String.h"
#include <image_transport/image_transport.h>
#include "sensor_msgs/Range.h"
#include "sensor_msgs/Illuminance.h"
#include <geometry_msgs/Twist.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#define FORWARD 0
#define TWIST 1
#define SPEED 0.3
#define PI4 0.78539816339


float illuR, illuL, range;


void driver(int order){

	ROS_INFO("ZE IN DRIVER BOIS");
    ros::NodeHandle nh; //Node's handler name is nh

  // Init cmd_vel publisher
  ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);

  // Create Twist message
  geometry_msgs::Twist twist;

	switch (order) {
		case FORWARD : 
			twist.linear.x = SPEED; //0.3 m/s
			twist.linear.y = 0;
    			twist.linear.z = 0;

    			twist.angular.x = 0;
    			twist.angular.y = 0;
			twist.angular.z = 0;
		break;
		case TWIST :
			twist.linear.x = 0;
			twist.linear.y = 0;
    			twist.linear.z = 0;

    			twist.angular.x = 0;
    			twist.angular.y = 0;
			twist.angular.z = PI4; //45°/Sec
			pub.publish(twist); //send command to robot actuators
			ros::spinOnce();
			usleep(1000000); //Sleep 1second (the robot should be turned by 45° somehow
			twist.linear.x = 0;
			twist.linear.y = 0;
    			twist.linear.z = 0;

    			twist.angular.x = 0;
    			twist.angular.y = 0;
			twist.angular.z = 0;
		break;
	}
	pub.publish(twist);//send command to robot actuators
	ros::spinOnce();
	return;
}


int explorer(){ //Some kind of god of war exploration
	ROS_INFO("ZE IN EXPLORER BOIS");
	if(illuR<255 || illuL<255 || 1){ //Threshold to be defined
		driver(FORWARD); //robot goes forward
		if(range<5){ //Threshold to be defined
			driver(TWIST); //robot turns 45° and stops
		}
	}
	//else //Fonction suivre ligne To be defined
return;
}

void rangeCallback(const sensor_msgs::Range::ConstPtr& msg)
{
  //ROS_INFO("Sonar Seq: [%d]", msg->header.seq);//Debug message console
  //ROS_INFO("Sonar Range: [%f]", msg->range);//Debug message console
  range = msg->range;
}

void illurCallback(const sensor_msgs::Illuminance::ConstPtr& msg)
{
  ROS_INFO("Illuminance Right Seq: [%d]", msg->header.seq);//Debug message console
  ROS_INFO("Illuminance Right Value: [%f]", msg->illuminance);//Debug message console
  illuR = msg->illuminance;
}

void illulCallback(const sensor_msgs::Illuminance::ConstPtr& msg)
{
  ROS_INFO("Illuminance Left Seq: [%d]", msg->header.seq);//Debug message console
  ROS_INFO("Illuminance Left Value: [%f]", msg->illuminance);//Debug message console
  illuL = msg->illuminance;
}

void camrCallback(const sensor_msgs::Image::ConstPtr& msg){
  ROS_INFO("Checking on right IR sensor");//Debug message console
}

void camlCallback(const sensor_msgs::Image::ConstPtr& msg){
  ROS_INFO("Checking on left IR sensor");//Debug message console
}



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

  ros::MultiThreadedSpinner spinner(2); // Use 4 threads
  

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
  ros::Subscriber subson = n.subscribe("/sensor/ir_front", 1000, rangeCallback); //Subscribe to this topic, ????, Use this fonction when something is published on this topic
  ros::Subscriber subIRR = n.subscribe("/light_sensor_pluginD/lightSensorD", 1000, illurCallback);
  ros::Subscriber subIRL = n.subscribe("/light_sensor_pluginL/lightSensorL", 1000, illulCallback);
  ros::Subscriber subcamR = n.subscribe("/cameraR/rgb/image_raw", 1000, &camrCallback);
  ros::Subscriber subcamL = n.subscribe("/cameraL/rgb/image_raw", 1000, &camlCallback);
  explorer();
  /**
   * ros::spin() will enter a loop, pumping callbacks.  With this version, all
   * callbacks will be called from within this thread (the main one).  ros::spin()
   * will exit when Ctrl-C is pressed, or the node is shutdown by the master.
   */
  ros::spin();

  return 0;
}
