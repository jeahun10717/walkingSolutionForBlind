#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud2_iterator.h>
#include <sensor_msgs/LaserScan.h>
#include <cmath>
#include <math.h>
#include <iostream>
#include <typeinfo>

using namespace std;

// 메시지 콜백 함수 선언
//// 처리할 메시지를 포인터로서 파라미터로 전달. 포인터의 자료형은 메시지의 타입을 명시
void scanCallback(sensor_msgs::PointCloud2ConstPtr const& msg)
{
    system("clear");
    int a = 0;
    double distance = 0;
    double distSum = 0;
    int count = 0;
    int zeroCnt = 0;
    int roopCnt = 0;
    for (sensor_msgs::PointCloud2ConstIterator<float> it(*msg, "x"); it != it.end(); ++it) {
        cout << fixed;
        cout.precision(3);
        // TODO: do something with the values of x, y, z
        distance = sqrt(it[0] * it[0] + it[1] * it[1] + it[2] * it[2]);
        int itrow = roopCnt / 60;
        int itcol = roopCnt % 60;

        // cout << itrow << ' ' << itcol << '\n';
        if(itrow >= 75 && itrow < 85 && itcol >= 25 && itcol < 35){
            cout << distance << ' ';
            if((roopCnt + 1) % 10 == 0) cout << '\n';
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
    // cout << distSum / zeroCnt << ", " << count / (double)(9600 - zeroCnt) * 100 << '\n';

    double dist = distSum / (double)(100 - zeroCnt);

    // if(isnan(dist)){
    //     cout << "->" << distSum << ' ' << (double)(200 - zeroCnt) << ' ' << zeroCnt << '\n';
    // }
    if(zeroCnt != 100) cout << dist << "\n";
    else cout << -1 << '\n';
    // system("clear");

}

int main(int argc, char **argv)
{
//   ros::init(argc, argv, "listener");		// 노드 이름 초기화

//   ros::NodeHandle nh;

//   // 서브스크라이버 선언
//   //// 토픽 이름: chatter(publish 노드에서 주는 토픽), 큐 사이즈: 100개
//   //// 콜백 함수 이름: chatterCallback(위에서 정의함, 여기서 받은 토픽을 처리함)
//   ros::Subscriber sub = nh.subscribe("/scan_3D", 100, chatterCallback);

//   ros::spin();		// 큐에 요청된 콜백함수를 처리하며, 프로그램 종료시까지 반복함

//   return 0;
    ros::init(argc, argv, "pointcloud2_subscriber");
    ros::NodeHandle nh;

    ros::Subscriber scan_sub = nh.subscribe<sensor_msgs::PointCloud2>("scan_3D", 5, scanCallback);

    ros::spin();

    return 0;
}