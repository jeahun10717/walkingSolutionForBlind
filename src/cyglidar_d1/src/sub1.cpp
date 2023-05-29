#include <ros/ros.h>
#include <string>
#include <std_msgs/String.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud2_iterator.h>
#include <sensor_msgs/LaserScan.h>
#include <cmath>
#include <math.h>
#include <iostream>
#include <typeinfo>
#include <tuple>
#include <alsa/asoundlib.h>

using namespace std;

int spinCnt = 0;
int roopCnt;

void publishMessage(ros::Publisher& publisher, const std::string& message) {
    std_msgs::String msg;
    msg.data = message;

    // 메시지 발행
    publisher.publish(msg);
}

// 메시지 콜백 함수 선언
//// 처리할 메시지를 포인터로서 파라미터로 전달. 포인터의 자료형은 메시지의 타입을 명시
void scanCallback(sensor_msgs::PointCloud2ConstPtr const& msg, ros::Publisher& publisher)
{
    roopCnt = 0;

    system("clear");
    int a = 0;
    double distance = 0;
    double distSum = 0;
    int count = 0;
    int zeroCnt = 0;
    double pointArr[60][160];
    double pcArr[60][160];
    tuple<double, double, double> point_tup_arr[60][160];
    for (sensor_msgs::PointCloud2ConstIterator<float> it(*msg, "x"); it != it.end(); ++it) {
        cout << fixed;
        cout.precision(2);
        // TODO: do something with the values of x, y, z
        distance = sqrt(it[0] * it[0] + it[1] * it[1] + it[2] * it[2]);
        int itrow = roopCnt / 160;
        int itcol = roopCnt % 160;
        pointArr[itrow][itcol] = distance;

        int itpcRow = roopCnt % 60;
        int itpcCol = roopCnt / 60;
        pcArr[itpcRow][itpcCol] = distance;
        
        point_tup_arr[itrow][itcol] = make_tuple(it[0], it[1], it[2]);

        // cout << itrow << ' ' << itcol << '\n';
        if(itrow >= 75 && itrow < 85 && itcol >= 25 && itcol < 35){
            // cout << distance << ' ';
            // if((roopCnt + 1) % 10 == 0) cout << '\n';
            if(distance != 0){
                // cout << "distplus" << '\n';
                distSum += distance;
            }else{
                // cout << "@@@@" << '\n';
                zeroCnt++;
            }
        }
        // if(distance <= 0.5 && distance != 0){
        //     count++;
        // }
        // if(distance == 0){
        //     zeroCnt++;
        // }
        // if(distance != 0){
        //     distSum += distance;
        // }
        roopCnt++;
    }
    
    double transArr[60][160];
    cout << "========================================================\n";
    spinCnt++;
    cout << "spin 횟수 : " << spinCnt << '\n';
    cout << "왼쪽에서 오른쪽 순으로 들어온다고 가정한 행렬 \n";
    for(int i = 0; i < 60; i++){
        for(int j = 0; j < 160; j++){
            if(i%2==0 && j%2==0) cout << pointArr[i][j] << ' ';
            // transArr[j][i] = pointArr[i][j];
        }   
        if(i%2==0) cout << '\n';
    }

    for(int i = 0; i < 60; i++){
        for(int j = 0; j < 160; j++){
            if(i%6==0 && j%16==0) 
                cout << '(' << get<0>(point_tup_arr[i][j]) << ',' << get<1>(point_tup_arr[i][j]) << ',' << get<2>(point_tup_arr[i][j]) << ") ";
        }   
        if(i%6==0) cout << '\n';
    }

    // cout << "위에서 아래 순으로 들어온다고 가정한 행렬 \n";
    // for(int i = 0; i < 60; i++){
    //     for(int j = 0; j < 160; j++){
    //         if(i%2 == 0 && j%4 == 0)cout << pcArr[i][j] << ' ';
    //     }   
    //     if(i%2 == 0)cout << '\n';
    // }
    double dist = distSum / (double)(100 - zeroCnt);
    std::string pubMsg = to_string(spinCnt) + '\n';

    publishMessage(publisher, pubMsg);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "pointcloud2_subscriber");
    ros::NodeHandle nh;
    

    ros::Publisher publisher = nh.advertise<std_msgs::String>("sub_to_pub", 10);
    ros::Subscriber scan_sub = nh.subscribe<sensor_msgs::PointCloud2>("scan_3D", 5, boost::bind(scanCallback, _1, publisher));

    ros::spin();

    return 0;
}