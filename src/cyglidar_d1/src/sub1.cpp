#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud2_iterator.h>
#include <sensor_msgs/LaserScan.h>
#include <cmath>
#include <math.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <typeinfo>
#include <tuple>
#include <alsa/asoundlib.h>
#include <thread>
#include <stdlib.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;
int soundCnt = 0;
int spinCnt = 0;
int x_area = 0;
int max_i = 0;
string filePath = "/home/jetson1/cyglidar_ws/src/cyglidar_d1/sdk/D1_ROS1/warning_sound.wav";

const char* device = "default";  // ALSA 장치 이름
unsigned int sampleRate = 44100;  // 샘플 레이트 (Hz)
unsigned int duration = 500;  // 재생 시간 (밀리초)
unsigned int channelCount = 2; // 채널 수
unsigned int bytesPerSample = 16; // bps

// struct WAVHeader {
//     char riff[4];           // "RIFF"
//     unsigned int fileSize;  // 파일 크기 - 8
//     char wave[4];           // "WAVE"
//     char fmt[4];            // "fmt "
//     unsigned int fmtSize;   // 포맷 청크 크기
//     // 새로운 필드 추가
//     unsigned short audioFormat;  // 오디오 포맷 (1은 PCM)
//     unsigned short numChannels;  // 채널 수
//     unsigned int sampleRate;     // 샘플 레이트
//     unsigned int byteRate;       // 바이트 레이트
//     unsigned short blockAlign;   // 블록 얼라인
//     unsigned short bitsPerSample;  // 샘플 당 비트 수
// };
ifstream file(filePath, ios::binary); 
void playSound(snd_pcm_t* handle, unsigned int sampleRate, unsigned int duration, unsigned int bufferSize ,int LR) {
    int err;
    int inputDataLeft = 0;
    int inputDataRight = 0;
    // if(LR < 0){
    //     inputDataRight = 0;
    //     inputDataLeft = 0;   
    // }
    if(LR == 0){
        inputDataRight = 100;
        inputDataLeft = 1;
    }
    else if(LR == 1){
        inputDataRight = 100;
        inputDataLeft = 1;   
    }
    else if(LR == 2){
         inputDataRight = 100;
         inputDataLeft = 1;   
    }
    else if(LR == 3){
        inputDataRight = 50;
        inputDataLeft = 50;   
    }
    else if(LR == 4){
        inputDataRight = 1;
        inputDataLeft = 100;   
    }
    else if(LR == 5){
        inputDataRight = 1;
        inputDataLeft = 100;   
    }
    
    // 재생할 샘플 수 계산
    unsigned int sampleCount = sampleRate * duration / 1000;

    // 사운드 데이터 생성
    short buffer[bufferSize];
    for (unsigned int i = 0; i < sampleCount; ++i) {//나중에 확인
        // 사운드 데이터 생성 코드 작성
        double t = (double)i / sampleRate;
        double frequencyLeft = 1760.0;
        double frequencyRight = 1760.0;
        double amplitude = 1.0;

        double valueLeft = amplitude * sin(2 * M_PI * frequencyLeft * t);
        double valueRight = amplitude * sin(2 * M_PI * frequencyRight * t);

        // 입력된 데이터의 크기에 따라 소리의 크기 조절
        int inputDataL = inputDataLeft;  // 예시로 데이터 값 설정 (0~255 범위)
        int inputDataR = inputDataRight;  // 예시로 데이터 값 설정 (0~255 범위)
        double scaleFactorL = (inputDataL / frequencyLeft);  // 데이터 크기에 따른 스케일 팩터 계산
        double scaleFactorR = (inputDataR / frequencyRight);  // 데이터 크기에 따른 스케일 팩터 계산

        ////좌우
        //buffer[2 * (i % bufferSize)] = static_cast<short>(valueLeft* scaleFactorL * SHRT_MAX);
        //buffer[2 * (i % bufferSize) + 1] = static_cast<short>(valueRight* scaleFactorR * SHRT_MAX);
        //좌우
        buffer[(i % bufferSize)] = static_cast<short>(valueLeft* scaleFactorL * SHRT_MAX);
        buffer[(i % bufferSize) + 1] = static_cast<short>(valueRight* scaleFactorR * SHRT_MAX);

        // 버퍼가 가득 차면 재생
        if ((i + 1) % bufferSize == 0) {
            if ((err = snd_pcm_writei(handle, buffer, bufferSize)) != bufferSize) {
                if (err < 0) {
                    std::cout << "오디오 재생 중 오류 발생: " << snd_strerror(err) << std::endl;
                } else {
                    std::cout << "오디오 재생 중 오류 발생: 기대한 샘플 수와 다릅니다." << std::endl;
                }
                break;
            }
        }
    }

    // 나머지 버퍼 재생
    unsigned int remainder = sampleCount % bufferSize;
    if (remainder > 0) {
        if ((err = snd_pcm_writei(handle, buffer, remainder)) != remainder) {
            if (err < 0) {
                std::cout << "오디오 재생 중 오류 발생: " << snd_strerror(err) << std::endl;
            } else {
                std::cout << "오디오 재생 중 오류 발생: 기대한 샘플 수와 다릅니다." << std::endl;
            }
        }
    }
}


void threadFunction()
{
    snd_pcm_t* handle;
    int err;
    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        std::cout << "오디오 장치를 열 수 없습니다: " << snd_strerror(err) << std::endl;
        return ;
    }
    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, channelCount);
    snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, 0);
    snd_pcm_hw_params(handle, params);

    unsigned int bufferSize = sampleRate * channelCount * bytesPerSample * (duration / 1000.0);
    // system("aplay /home/jetson1/cyglidar_ws/src/cyglidar_d1/sdk/D1_ROS1/warning_sound.wav");


    playSound(handle, sampleRate, duration, bufferSize, max_i );
    soundCnt++;

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
 
}
void threadFunction2()
{
    // 스레드에서 실행될 코드 작성
    //system("canberra-gtk-play -f /home/jetson1/cyglidar_ws/src/cyglidar_d1/sdk/D1_ROS1/warning_sound1.wav");
    //std::cout << "스레드 실행" << std::endl;
    // for(int i = 0 ;i < 3; i++){
   //     cout << "aa" << '\n';
    //     sleep(3);
    // ros::spin();
   // }
    
}


void scanCallback(sensor_msgs::PointCloud2ConstPtr const& msg)
{   
    system("clear");
    int a = 0;
    double distance = 0;
    double distSum = 0;
    int count = 0;
    int zeroCnt = 0;
    int roopCnt = 0;
    double pointArr[60][160];
    double pcArr[60][160];
    tuple<double, double, double> point_tup_arr[60][160];

    for (sensor_msgs::PointCloud2ConstIterator<float> it(*msg, "x"); it != it.end(); ++it) {
        cout << fixed;
        cout.precision(2);
        distance = sqrt(it[0] * it[0] + it[1] * it[1] + it[2] * it[2]);

        int itrow = roopCnt / 160;
        int itcol = roopCnt % 160;
        
        point_tup_arr[itrow][itcol] = make_tuple(it[0], it[1], it[2]);

        
        roopCnt++;
    }
    
    double transArr[60][160];
    
        
    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int count4 = 0;
    int count5 = 0;
    int count1_arr[6] = { 0 };
    int count2_arr[6] = { 0 };
    int count3_arr[6] = { 0 };
    int count4_arr[6] = { 0 };
    int zerocccount = 0;
  
    for(int i = 0; i < 60; i++){
        for(int j = 0; j < 160; j++){

          double x = get<0>(point_tup_arr[i][j]);
          double y = get<1>(point_tup_arr[i][j]);
          double z = get<2>(point_tup_arr[i][j]);
            if( x == 0 && y == 0 && z == 0){
                zerocccount++;
            }

            else if( x <= 0.5 && (y >= -0.6 && y <= 0.6) && (z >= -0.5 && z <= 0.5 ) ){  
                if(y >= -0.6 && y < -0.4){
                    count1_arr[0]++;
                }  
                else if(y >= -0.4 && y < -0.2){
                    count1_arr[1]++;
                }
                else if(y >= -0.2 && y < 0){
                    count1_arr[2]++;
                }
                else if(y >= 0 && y < 0.2){
                    count1_arr[3]++;
                }
                else if(y >= 0.2 && y < 0.4){
                    count1_arr[4]++;
                }
                else if(y >= 0.4 && y < 0.6){
                    count1_arr[5]++;
                }

                count1++; 
            }
            else if( x <= 1.0 && (y >= -0.6 && y <= 0.6) && (z >= -0.5 && z <= 0.5 ) ){
                if(y >= -0.6 && y < -0.4){
                    count2_arr[0]++;
                }  
                else if(y >= -0.4 && y < -0.2){
                    count2_arr[1]++;
                }
                else if(y >= -0.2 && y < 0){
                    count2_arr[2]++;
                }
                else if(y >= 0 && y < 0.2){
                    count2_arr[3]++;
                }
                else if(y >= 0.2 && y < 0.4){
                    count2_arr[4]++;
                }
                else if(y >= 0.4 && y < 0.6){
                    count2_arr[5]++;
                }
                count2++;
            }
            else if( x <= 1.5 && (y >= -0.6 && y <= 0.6) && (z >= -0.5 && z <= 0.5 ) ){
                
                if(y >= -0.6 && y < -0.4){
                    count3_arr[0]++;
                }  
                else if(y >= -0.4 && y < -0.2){
                    count3_arr[1]++;
                }
                else if(y >= -0.2 && y < 0){
                    count3_arr[2]++;
                }
                else if(y >= 0 && y < 0.2){
                    count3_arr[3]++;
                }
                else if(y >= 0.2 && y < 0.4){
                    count3_arr[4]++;
                }
                else if(y >= 0.4 && y < 0.6){
                    count3_arr[5]++;
                }
                    count3++;
                    
            }
            else if( x <= 2.0 && (y >= -0.6 && y <= 0.6) && (z >= -0.5 && z <= 0.5 ) ){
                    count4++;
                if(y >= -0.6 && y < -0.4){
                    count4_arr[0]++;
                }  
                else if(y >= -0.4 && y < -0.2){
                    count4_arr[1]++;
                }
                else if(y >= -0.2 && y < 0){
                    count4_arr[2]++;
                }
                else if(y >= 0 && y < 0.2){
                    count4_arr[3]++;
                }
                else if(y >= 0.2 && y < 0.4){
                    count4_arr[4]++;
                }
                else if(y >= 0.4 && y < 0.6){
                    count4_arr[5]++;
                }
            }
            else {
                count5++;
            }
        }  
   }
    
    tuple<int,double> t1;
    // int x_area;
    double LR_area;
    // int max_i = 0;

    if(count1 / (9600.0 - zerocccount) * 100 > 40.0 ){
        cout << "경고 0" << '\n';
        x_area = 0;
        int max = count1_arr[0];
        max_i = 0;// 
        for(int i= 0 ; i < 6; i++){
            cout << i << "번구역 : " << count1_arr[i] / (double)count1 * 100 << '\n';
            if(count1_arr[i] > max){
                max = count1_arr[i];
                max_i = i;
            }
        }
        cout << "max : " << max / (double)count1 * 100 << '\n';
        cout << "i : "<< max_i << '\n';
        
    }
    
    else if(count2 / (9600.0 - zerocccount) * 100 > 30.0 ){
        cout << "경고 1" << '\n';
        x_area = 1;
        int max = count2_arr[0];
        max_i = 0; 
        for(int i= 0 ; i < 6; i++) {
            cout << i << "번구역 : " << count2_arr[i] / (double)count2 * 100 << '\n';
            if(count2_arr[i] > max){
                max = count2_arr[i];
                max_i = i;
            }
        }
        cout << "max : " << max / (double)count2 * 100 << '\n';
        cout << "i : "<< max_i << '\n';
    }
    
    else if(count3 / (9600.0 - zerocccount) * 100 > 20.0 ){
        cout << "경고 2" << '\n';
        x_area = 2;
        int max = count3_arr[0];
        max_i = 0; 

        for(int i= 0 ; i < 6; i++){
            cout << i << "번구역 : " << count3_arr[i] / (double)count3 * 100 << '\n';
            if(count3_arr[i] > max){
                max = count3_arr[i];
                max_i = i;
            }
        }
        cout << "max : " << max / (double)count3 * 100 << '\n';
        cout << "i : "<< max_i << '\n';
    }

    else if(count4 / (9600.0 - zerocccount) * 100 > 10.0 ){
        cout << "경고 3" << '\n';
        x_area = 3;
        int max = count4_arr[0];
        // max_i = 0; 
        for(int i= 0 ; i < 6; i++){
            cout << i << "번구역 : " << count4_arr[i] / (double)count4 * 100 << '\n';
            if(count4_arr[i] > max){
                max= count4_arr[i];
                max_i = i;
            }
        }
        cout << "max : " << max / (double)count4 * 100 << '\n';
        cout << "i : "<< max_i << '\n';
    }
    else {
        cout << "경고 무" <<"\n";
        x_area = -1;
        max_i = -1;
    }  
    
    // string filePath = "/home/jetson1/walkingSolutionForBlind_sound/warning_sound1.wav";
    double volumeBalance = 0.5;  // 좌우 소리 균형 값 (0.0은 완전 왼쪽, 1.0은 완전 오른쪽)
    
    cout << "========================================================\n";
    spinCnt++;
    cout << "spin 횟수 : " << spinCnt << '\n';
    cout << "x area : " << x_area << '\n';
    cout << "y area : " << max_i << '\n';
    cout << "soundCnt : " << soundCnt <<'\n';
    

    if(spinCnt % 5 == 0 ){
        thread th1(threadFunction);
        th1.detach();
    }
    
    

}

    


int main(int argc, char **argv)
{   

    // playSound(handle, sampleRate, duration, bufferSize);

    ros::init(argc, argv, "pointcloud2_subscriber");
    ros::NodeHandle nh;

    ros::Subscriber scan_sub = nh.subscribe<sensor_msgs::PointCloud2>("scan_3D", 5, scanCallback);
    // ros::Rate rate(2);

    // while(ros::ok()){
    //     ros::spinOnce();
    //     rate.sleep();
    // }
    // thread th1(threadFunction);
    // thread th2(threadFunction2);
    // th1.join();        
    // th2.join();
    ros::spin();
    
    // snd_pcm_drain(handle);
    // snd_pcm_close(handle);

    return 0;


}