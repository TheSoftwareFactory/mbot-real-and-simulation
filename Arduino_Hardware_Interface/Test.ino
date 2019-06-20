#include <ros.h>
#include <ros/time.h>
#include <std_msgs/UInt16.h>
#include <geometry_msgs/Twist.h>
#include <rosserial_arduino/Adc.h>
#include <math.h>

#define DISTANCEWHEELS 0.09

float motorSpeedr = 0;
float motorSpeedl = 0;
float motorSpeed = 0;
float alpha = 0;
float delta = 0;

void motorCallBack(const geometry_msgs::Twist& vel_msg)
{
  motorSpeed = (vel_msg.linear.x); //Si linear x en m/s AVEC UN RAPPORT K
  delta = RADIUSWHEEL*(vel_msg.angular.z)/DISTANCEWHEELS; //l * rad/sec / r
  motorSpeedr = motorSpeed - vel_msg.angular.z*DISTANCEWHEELS/2;
  motorSpeedl = motorSpeed + vel_msg.angular.z*DISTANCEWHEELS/2;
}

ros::NodeHandle  nh;
geometry_msgs::Twist vel_msg;    //receiving motor speed(pwm)
std_msgs::UInt16 illur_msg;      //sending illuminance from right sensor value
std_msgs::UInt16 illul_msg;      //sending illuminance from left sensor value
std_msgs::Uint16 range_msg      //sending range value
ros::Publisher pub_illur( "/light_sensor_pluginD/lightSensorD", &illur_msg);
ros::Publisher pub_illul( "/light_sensor_pluginL/lightSensorL", &illul_msg);
ros::Publisher pub_range( "/sensor/ir_front", &light_msg);
ros::Subscriber <geometry_msgs::Twist> sub("/cmd_vel", &motorCallBack);

void setup() {
  // put your setup code here, to run once:
  nh.initNode();
  nh.subscribe(sub);
  nh.advertise(pub_illur);
  nh.advertise(pub_illul);
  nh.advertise(pub_range);
   
  pinMode(4,OUTPUT);    //R_motor direction
  pinMode(5,OUTPUT);    //R_motor pwm
  pinMode(6,OUTPUT);    //L_motor pwm
  pinMode(7,OUTPUT);    //L_motor direction
  //pinMode(A6,INPUT);    //Light_Sensor input
  //pinMode(A6,INPUT);    //illur pin setup
  //pinMode(A6,INPUT);    //illul pin setup
  digitalWrite(4, HIGH);
  digitalWrite(7, HIGH);
  analogWrite(5, motorSpeed);
  analogWrite(6, motorSpeed);
}

void loop() {
  // put your main code here, to run repeatedly:
  //publish the adc value every 50 milliseconds
  //since it takes that long for the sensor to stablize
  illur_msg.data = analogRead(); //read from IR RIGHT
  illul_msg.data = analogRead(); //read from IR LEFT
  range_msg.data = analogRead(); //read from SONAR
  pub_light.publish(&illur_msg);
  pub_light.publish(&illul_msg);
  pub_light.publish(&range_msg);
  
  analogWrite(5, motorSpeedr);
  analogWrite(6, motorSpeedl);
  nh.spinOnce();
}
