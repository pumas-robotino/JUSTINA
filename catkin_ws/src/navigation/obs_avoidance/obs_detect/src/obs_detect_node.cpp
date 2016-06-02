#include <iostream>
#include <cmath>
#include "ros/ros.h"
#include "std_msgs/Bool.h"
#include "nav_msgs/Path.h"
#include "sensor_msgs/LaserScan.h"
#include "geometry_msgs/PoseStamped.h"
#include "tf/transform_listener.h"

sensor_msgs::LaserScan laserScan;
nav_msgs::Path lastPath;
int currentPathIdx = 0;

void callbackLaserScan(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    laserScan = *msg;
}

void callbackPath(const nav_msgs::Path::ConstPtr& msg)
{
    lastPath = *msg;
    currentPathIdx = 20;
}

int main(int argc, char** argv)
{
    std::cout << "INITIALIZING OBSTACLE DETECTOR (ONLY LASER) NODE BY MARCOSOFT... " << std::endl;
    ros::init(argc, argv, "obs_detect");
    ros::NodeHandle n;
    ros::Subscriber subLaserScan = n.subscribe("/hardware/scan", 1, callbackLaserScan);
    ros::Subscriber subPath = n.subscribe("/navigation/mvn_pln/last_calc_path", 1, callbackPath);
    ros::Publisher pubObstacleInFront = n.advertise<std_msgs::Bool>("/navigation/obs_avoid/obs_in_front", 1);
    ros::Publisher pubCollisionRisk = n.advertise<std_msgs::Bool>("/navigation/obs_avoid/collision_risk", 1);
    tf::TransformListener tf_listener;
    ros::Rate loop(10);

    std_msgs::Bool msgObsInFront;
    std_msgs::Bool msgCollisionRisk;
    
    tf::StampedTransform tf;
    tf::Quaternion q;

    bool obsInFront = false;
    bool collisionRisk = false;
    float minSearchAngle = 0;  //This angles will be determined according to the point which
    float maxSearchAngle = 0;  //is 1.0 [m] (20 path-steps) ahead the robot. They are used for determining
    float searchDistance = 0.7;//collision risk. ObsInFront is calculated always with the same angles
    int searchMinCounting = 0;
    int searchCounter = 0;
    float robotX = 0;
    float robotY = 0;
    float robotTheta = 0;
    int obsInFrontCounter = 0;
    
    laserScan.angle_increment = 3.14/512.0; //Just to have something before the first callback
    lastPath.poses.push_back(geometry_msgs::PoseStamped()); //Just to have something before the first callback
    tf_listener.waitForTransform("map", "base_link", ros::Time(0), ros::Duration(5.0));

    while(ros::ok())
    {
        //Getting robot position
        tf_listener.lookupTransform("map", "base_link", ros::Time(0), tf);
        robotX = tf.getOrigin().x();
        robotY = tf.getOrigin().y();
        q = tf.getRotation();
        robotTheta = atan2((float)q.z(), (float)q.w()) * 2;
        //Calculating position 20 path-steps ahead the robot
        if(currentPathIdx > (lastPath.poses.size()-1))
            currentPathIdx = lastPath.poses.size() - 1;
        float lookAheadX = lastPath.poses[currentPathIdx].pose.position.x;
        float lookAheadY = lastPath.poses[currentPathIdx].pose.position.y;
        float distToNextPose = sqrt((lookAheadX - robotX)*(lookAheadX - robotX) + (lookAheadY - robotY)*(lookAheadY - robotY));
        float lookAheadAngle = atan2(lookAheadY - robotY, lookAheadX - robotX) - robotTheta;
        if(lookAheadAngle > M_PI) lookAheadAngle -= 2*M_PI;
        if(lookAheadAngle <= -M_PI) lookAheadAngle += 2*M_PI;
        searchDistance = distToNextPose;
        if(searchDistance > 0.7) searchDistance = 0.7;
        if(searchDistance < 0.15) searchDistance = 0.15;
        minSearchAngle = lookAheadAngle - 0.6/searchDistance/2; //Search angle is calculated such that, at the given search distance,
        maxSearchAngle = lookAheadAngle + 0.6/searchDistance/2; //an arc of 0.6m (a litle bit more than the robot width) is covered
        if(minSearchAngle > M_PI) minSearchAngle -= 2*M_PI;
        if(minSearchAngle <= -M_PI) minSearchAngle += 2*M_PI;
        if(maxSearchAngle > M_PI) maxSearchAngle -= 2*M_PI;
        if(maxSearchAngle <= -M_PI) maxSearchAngle += 2*M_PI;
        searchMinCounting = (int)(0.6/searchDistance/laserScan.angle_increment*0.25); //I think (but I'm not sure) this will detect a 0.15m sized object
        //Checking for obstacles in front and collisions
        obsInFrontCounter = 0;
        searchCounter = 0;
        for(int i=0; i < laserScan.ranges.size(); i++)
        {
            float currentAngle = laserScan.angle_min + i*laserScan.angle_increment;
            if(laserScan.ranges[i] < 0.45 && currentAngle > -0.6 && currentAngle < 0.6)
                obsInFrontCounter++;
            if(laserScan.ranges[i] < searchDistance && currentAngle > minSearchAngle && currentAngle < maxSearchAngle)
                searchCounter++;
        }
        obsInFront = obsInFrontCounter > 60;
        collisionRisk = searchCounter > searchMinCounting;
        if(obsInFront)
            std::cout << "ObsDetector.->Obstacle in front!!!" << std::endl;
        //Moving currentPathIdx to always point 1m ahead
        while(distToNextPose < 1.0 && ++currentPathIdx < lastPath.poses.size())
        {
            lookAheadX = lastPath.poses[currentPathIdx].pose.position.x;
            lookAheadY = lastPath.poses[currentPathIdx].pose.position.y;
            distToNextPose = sqrt((lookAheadX - robotX)*(lookAheadX - robotX) + (lookAheadY - robotY)*(lookAheadY - robotY));
        }
        //Publishing if there is an obstacle 20 path-steps ahead the robot (possible collision)
        msgCollisionRisk.data = collisionRisk;
        pubCollisionRisk.publish(msgCollisionRisk);
        //Publishing if there is an obstacle in front of the robot
        msgObsInFront.data = obsInFront;
        pubObstacleInFront.publish(msgObsInFront);
        ros::spinOnce();
        loop.sleep();
    }
}
