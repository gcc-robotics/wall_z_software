#include <iostream>
#include <stdio.h>

#include "ros/ros.h"
#include "nav_msgs/OccupancyGrid.h"
#include "nav_msgs/MapMetaData.h"
#include "geometry_msgs/Pose.h"
#include "geometry_msgs/Point.h"
#include "geometry_msgs/Quaternion.h"

#include <cmath>

#define MAPRESOLUTION (0.025)

ros::Publisher occupancyDataPublisher;
ros::Publisher mapMetaDataPublisher;

int seq = 0;
int fake_occupancy_grid[1440000];

void  simulatedObstacle(double x, double y, double width, double height)
{
	int startCol = fabs(-15 - x) / MAPRESOLUTION;
	int startRow = fabs(15 - y - height) / MAPRESOLUTION;

	if(startCol < 0)
	{
		startCol = 0;
	}
	else if(startCol > 1199)
	{
		startCol = 1199;
	}

	if(startRow < 0)
	{
		startRow = 0;
	}
	else if(startRow > 1199)
	{
		startRow = 1199;
	}

	int endCol = (startCol + (width / MAPRESOLUTION) > 1199) ? 1199 : startCol + (width / MAPRESOLUTION);
	int endRow = (startRow + (height / MAPRESOLUTION) > 1199) ? 1199 : startRow + (height / MAPRESOLUTION);

	int index = 0;
	int currentCol = 0;

	ROS_INFO("%i, %i, %i, %i", startCol, startRow, endCol, endRow);

	for(; startRow < endRow; startRow++)
	{
		currentCol = startCol;

		for(; currentCol < endCol; currentCol++)
		{
			index = (startRow * 1200) + currentCol;
			fake_occupancy_grid[index] = 100;
		}
	}
}

int main(int argc, char **argv)
{
	ROS_INFO("Starting simulated_slam_node");

	ros::init(argc, argv, "simulated_slam_node");
	ros::NodeHandle node;

	ros::Publisher occupancyDataPublisher = node.advertise<nav_msgs::OccupancyGrid>("/map", 1);
	ros::Publisher mapMetaDataPublisher = node.advertise<nav_msgs::MapMetaData>("/map_metadata", 1);

	ros::Rate loopRate(10);

	// Create ROS messages
	nav_msgs::OccupancyGrid occupancy_grid_msg;
	nav_msgs::MapMetaData mapmetadata_msgs;

	occupancy_grid_msg.header.frame_id = "map";
	occupancy_grid_msg.data.resize(1440000); //1440000

	// row major order
	// i = row number
	// j = column number
	int index = 0;

	for(int i = 0; i < 1200; i++)
	{
		for(int j = 0; j < 1200; j++)
		{
			index = (i * 1200) + j;
			if(i == 0 || i == 1199 || j == 0 || j == 1199)
			{
				fake_occupancy_grid[index] = 100;
			}
			else
			{
				fake_occupancy_grid[index] = 0;
			}
		}
	}

	/*for(int i = 0; i < 1200; i++)
	{
		for(int j = 0; j < 1200; j++)
		{
			if(i >= 400 && i <= 800 && j >= 400 && j <= 800)
			{
				index = (i * 1200) + j;
				fake_occupancy_grid[index] = -1;
			}
			else
			{
				fake_occupancy_grid[index] = -1;
			}
		}
	}*/

	simulatedObstacle(0, 0, 1, 1);
	//simulatedObstacle(4, 0.5, 1, 1);

	for(int i = 0; i < 1440000; i++)
	{
		occupancy_grid_msg.data[i] = fake_occupancy_grid[i];
	}

	float fake_resolution = 0.025;
	int fake_height = 1200;
	int fake_width = 1200;

	mapmetadata_msgs.resolution = fake_resolution;
	mapmetadata_msgs.height = fake_height;
	mapmetadata_msgs.width = fake_height;

	float fake_pose_point_x = -15;
	float fake_pose_point_y = -15;
	float fake_pose_point_z = 0;

	mapmetadata_msgs.origin.position.x = fake_pose_point_x;
	mapmetadata_msgs.origin.position.y = fake_pose_point_y;
	mapmetadata_msgs.origin.position.z = fake_pose_point_z;

	float fake_pose_quaternion_x = 0;
	float fake_pose_quaternion_y = 0;
	float fake_pose_quaternion_z = 0;
	float fake_pose_quaternion_w = 1;

	mapmetadata_msgs.origin.orientation.x = fake_pose_quaternion_x;
	mapmetadata_msgs.origin.orientation.y = fake_pose_quaternion_y;
	mapmetadata_msgs.origin.orientation.z = fake_pose_quaternion_z;
	mapmetadata_msgs.origin.orientation.w = fake_pose_quaternion_w;

	occupancy_grid_msg.info = mapmetadata_msgs;

	while(ros::ok())
	{
		occupancy_grid_msg.header.stamp = ros::Time::now();
		occupancy_grid_msg.header.seq++;

		occupancyDataPublisher.publish(occupancy_grid_msg);
		mapMetaDataPublisher.publish(mapmetadata_msgs);

		ros::spinOnce();
		loopRate.sleep();
	}

	ros::shutdown();

	return 0;
}