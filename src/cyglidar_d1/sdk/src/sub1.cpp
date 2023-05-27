// #include <ros/ros.h>
// #include <sensor_msgs/PointCloud2.h>

// #include <pcl_conversions/pcl_conversions.h>

// // PCL specific includes
// #include <pcl/conversions.h>
// #include <pcl/filters/voxel_grid.h>
// #include <pcl/point_cloud.h>
// #include <pcl/point_types.h>

// void cloud_callback (const sensor_msgs::PointCloud2ConstPtr& cloud_msg){
//  ROS_INFO("inside callback");
// }

// int main (int argc, char** argv) {
//  ros::init (argc, argv, "cloud_sub");
//  ros::NodeHandle nh;
//  ros::Rate loop_rate(10);
//  ros::Subscriber sub;
//  while (nh.ok()) {
//    sub = nh.subscribe ("input_cloud", 1, cloud_callback);
//    ros::spinOnce ();
//    loop_rate.sleep ();
//  }
// }