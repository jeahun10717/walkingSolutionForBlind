#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud2_iterator.h>
#include <sensor_msgs/LaserScan.h>
#include "std_msgs/String.h"
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
#include <string>

#include <atomic>
using namespace std;
std::atomic<bool> global_flag(false);

bool soundFlag = false;


void playDLSound();



void toggleFlag() {
    while (true) {
        std::cout << "Press 't' to toggle the flag, 'q' to quit: ";
        char input;
        std::cin >> input;

        if (input == '.') {
            global_flag.store(!global_flag.load());
            std::cout << "Flag toggled." << std::endl;
        } else if (input == 'q') {
            break; // 무한 루프 종료
        } else {
            std::cout << "Invalid input. Press 't' to toggle the flag, 'q' to quit." << std::endl;
        }
    }
}



void threadFunction();
int soundCnt = 0;
int spinCnt = 0;
int x_area = 0;
int max_i = 0;
double SF = 0;
string filePath = "/home/jetson1/cyglidar_ws/src/cyglidar_d1/sdk/D1_ROS1/warning_sound.wav";

bool flag = false;

const char* device = "default";  // ALSA 장치 이름
unsigned int sampleRate = 44100;  // 샘플 레이트 (Hz)
unsigned int duration = 500;  // 재생 시간 (밀리초)
unsigned int channelCount = 2; // 채널 수
unsigned int bytesPerSample = 16; // bps

ifstream file(filePath, ios::binary); 



void playSound(snd_pcm_t* handle, unsigned int sampleRate, unsigned int duration, unsigned int bufferSize ,int LR) {
    int err;
    int inputDataLeft = 0;
    int inputDataRight = 0;
    
    // cout << "max_i : "<<max_i << '\n';
    // cout << "LR : "<<LR << '\n';
    if(LR == 0){
        inputDataRight = 255;
        inputDataLeft = 0;
    }
    else if(LR == 1){
        inputDataRight = 180;
        inputDataLeft = 0;   
    }
    else if(LR == 2){
         inputDataRight = 100;
         inputDataLeft = 100;   
    }
    else if(LR == 3){
        inputDataRight = 100;
        inputDataLeft = 100;   
    }
    else if(LR == 4){
        inputDataRight = 0;
        inputDataLeft = 180;   
    }
    else if(LR == 5){
        inputDataRight = 0;
        inputDataLeft = 255;   
    }
    
    // 재생할 샘플 수 계산
    unsigned int sampleCount = sampleRate * duration / 1000;

    // 사운드 데이터 생성
    short buffer[bufferSize];
    for (unsigned int i = 1; i < sampleCount; ++i) {//나중에 확인
        // 사운드 데이터 생성 코드 작성
        double t = (double)i / sampleRate;
        double frequencyLeft = SF;
        double frequencyRight = SF;
        double amplitude = 1.0;

        double valueLeft = amplitude * sin(2 * M_PI * frequencyLeft * t);
        double valueRight = amplitude * sin(2 * M_PI * frequencyRight * t);

        // 입력된 데이터의 크기에 따라 소리의 크기 조절
        int inputDataL = inputDataLeft;  // 예시로 데이터 값 설정 (0~255 범위)
        int inputDataR = inputDataRight;  // 예시로 데이터 값 설정 (0~255 범위)
        double scaleFactorL = (inputDataL / frequencyLeft);  // 데이터 크기에 따른 스케일 팩터 계산
        double scaleFactorR = (inputDataR / frequencyRight);  // 데이터 크기에 따른 스케일 팩터 계산

        ////좌우
        buffer[2 * (i % bufferSize)] = static_cast<short>(valueLeft* scaleFactorL * SHRT_MAX);
        buffer[2 * (i % bufferSize) + 1] = static_cast<short>(valueRight* scaleFactorR * SHRT_MAX);
        //좌우
        //buffer[(i % bufferSize)] = static_cast<short>(valueLeft* scaleFactorL * SHRT_MAX);
        //buffer[(i % bufferSize) + 1] = static_cast<short>(valueRight* scaleFactorR * SHRT_MAX);

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


    playSound(handle, sampleRate, duration, bufferSize, max_i);
    soundCnt++;

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
 
}

void playChangeSound(snd_pcm_t* handle, unsigned int sampleRate, unsigned int duration, unsigned int bufferSize) {
    int err;
    
    // 재생할 샘플 수 계산
    unsigned int sampleCount = sampleRate * duration / 1000;

    // 사운드 데이터 생성
    short buffer[bufferSize];
    for (unsigned int i = 1; i < sampleCount; ++i) {//나중에 확인
        // 사운드 데이터 생성 코드 작성
        double t = (double)i / sampleRate;
        double frequencyLeft = SF;
        double frequencyRight = SF;
        double amplitude = 1.0;

        double valueLeft = amplitude * sin(2 * M_PI * frequencyLeft * t);
        double valueRight = amplitude * sin(2 * M_PI * frequencyRight * t);

        // 입력된 데이터의 크기에 따라 소리의 크기 조절
        int inputDataL = 180;  // 예시로 데이터 값 설정 (0~255 범위)
        int inputDataR = 180;  // 예시로 데이터 값 설정 (0~255 범위)
        double scaleFactorL = (inputDataL / frequencyLeft);  // 데이터 크기에 따른 스케일 팩터 계산
        double scaleFactorR = (inputDataR / frequencyRight);  // 데이터 크기에 따른 스케일 팩터 계산

        ////좌우
        buffer[2 * (i % bufferSize)] = static_cast<short>(valueLeft* scaleFactorL * SHRT_MAX);
        buffer[2 * (i % bufferSize) + 1] = static_cast<short>(valueRight* scaleFactorR * SHRT_MAX);
        //좌우
        //buffer[(i % bufferSize)] = static_cast<short>(valueLeft* scaleFactorL * SHRT_MAX);
        //buffer[(i % bufferSize) + 1] = static_cast<short>(valueRight* scaleFactorR * SHRT_MAX);

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

void changeSoundThreadFunction()
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


    playChangeSound(handle, sampleRate, duration, bufferSize);
    // soundCnt++;

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
 
}


int SF_list[] = {25,50,75,100,125,150,175,200,225,250};
// int SF_list[] = {170,180,190,200,210,220,230,240,255,260};
int sf_count = 0;
bool cs_flag = true;

void playDLSound(){
    SF = SF_list[sf_count];

    if(sf_count < 10){
        thread th(threadFunction);
        th.detach();
        sf_count++;
    }
}

int sf_ros_count = 0;
void scanCallback(sensor_msgs::PointCloud2ConstPtr const& msg)
{   
    
    if(!flag){
        return;
    }

    if(!cs_flag){
        std::cout << "@@@@@@@@@@@@@@@@@@ cs_flag = true ::::: " << sf_count << '\n';
        
        if(sf_ros_count % 25 == 0){
            SF = SF_list[sf_count];
            thread th(changeSoundThreadFunction);
            th.detach();
            sf_count--;
        }
        sf_ros_count++;
        
        if(sf_count == 0){
            std::cout << "@@@@@@@@@@@@@@@@@@ sf_count == 10\n";
            sf_count = 0;
            sf_ros_count = 0;
            cs_flag = true;
        }
        return;
    }
    // if(flag && soundFlag == false){
    //     playDLSound();
    //     soundFlag = true;
    //     std::cout << "DL to ROS\n";
    // }
    // system("clear");
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
    double maxYlen = 0.51;
    double minYlen = -0.51;
    double devideY = maxYlen / 3;
    for(int i = 0; i < 60; i++){
        for(int j = 0; j < 160; j++){
            // double maxYLen = 0.6;
            // double minYLen = -0.6;
            // double devideY = maxYLen / 3;
            double x = get<0>(point_tup_arr[i][j]);
            double y = get<1>(point_tup_arr[i][j]);
            double z = get<2>(point_tup_arr[i][j]);
            if( x == 0 && y == 0 && z == 0){
                zerocccount++;
            }

            else if( x <= 0.5 && (y >= minYlen && y <= maxYlen) && (z >= -0.5 && z <= 0.5 ) ){  
                if(y >= minYlen && y < minYlen + devideY){
                    count1_arr[0]++;
                }  
                else if(y >= minYlen + devideY && y < minYlen + devideY * 2){
                    count1_arr[1]++;
                }
                else if(y >= minYlen + devideY * 2 && y < 0){
                    count1_arr[2]++;
                }
                else if(y >= 0 && y < maxYlen - devideY * 2){
                    count1_arr[3]++;
                }
                else if(y >= maxYlen - devideY * 2 && y < maxYlen - devideY){
                    count1_arr[4]++;
                }
                else if(y >= maxYlen - devideY && y < maxYlen){
                    count1_arr[5]++;
                }

                count1++; 
            }
            else if( x <= 1.0 && (y >= minYlen && y <= maxYlen) && (z >= -0.5 && z <= 0.5 ) ){
                if(y >= minYlen && y < minYlen + devideY){
                    count2_arr[0]++;
                }  
                else if(y >= minYlen + devideY && y < minYlen + devideY * 2){
                    count2_arr[1]++;
                }
                else if(y >= minYlen + devideY * 2 && y < 0){
                    count2_arr[2]++;
                }
                else if(y >= 0 && y < maxYlen - devideY * 2){
                    count2_arr[3]++;
                }
                else if(y >= maxYlen - devideY * 2 && y < maxYlen - devideY){
                    count2_arr[4]++;
                }
                else if(y >= maxYlen - devideY && y < maxYlen){
                    count2_arr[5]++;
                }
                count2++;
            }
            else if( x <= 1.5 && (y >= minYlen && y <= maxYlen) && (z >= -0.5 && z <= 0.5 ) ){
                
                if(y >= minYlen && y < minYlen + devideY){
                    count3_arr[0]++;
                }  
                else if(y >= minYlen + devideY && y < minYlen + devideY * 2){
                    count3_arr[1]++;
                }
                else if(y >= minYlen + devideY * 2 && y < 0){
                    count3_arr[2]++;
                }
                else if(y >= 0 && y < maxYlen - devideY * 2){
                    count3_arr[3]++;
                }
                else if(y >= maxYlen - devideY * 2 && y < maxYlen - devideY){
                    count3_arr[4]++;
                }
                else if(y >= maxYlen - devideY && y < maxYlen){
                    count3_arr[5]++;
                }
                    count3++;
                    
            }
            else if( x <= 2.0 && (y >= minYlen && y <= maxYlen) && (z >= -0.5 && z <= 0.5 ) ){
                    count4++;
                if(y >= minYlen && y < minYlen + devideY){
                    count4_arr[0]++;
                }  
                else if(y >= minYlen + devideY && y < minYlen + devideY * 2){
                    count4_arr[1]++;
                }
                else if(y >= minYlen + devideY * 2 && y < 0){
                    count4_arr[2]++;
                }
                else if(y >= 0 && y < maxYlen - devideY * 2){
                    count4_arr[3]++;
                }
                else if(y >= maxYlen - devideY * 2 && y < maxYlen - devideY){
                    count4_arr[4]++;
                }
                else if(y >= maxYlen - devideY && y < maxYlen){
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

    if (spinCnt % 300 == 0){
        flag = !flag;
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    if(x_area == 0) SF = 650.0;
    else if(x_area == 1) SF = 500.0;
    else if(x_area == 2) SF = 350.0;
    else if(x_area == 3) SF = 200.0;

    if(spinCnt % (x_area+1) == 0){
        thread th1(threadFunction);
        th1.detach();
        // th1.join();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    // if(spinCnt % (x_area+1) == 0 && flag){
    //     thread th1(threadFunction);
    //     th1.detach();
    //     // th1.join();
    //     // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // }
}

struct WavHeader {
    char riff[4];                // 'RIFF'
    unsigned int overallSize;    // 파일 크기
    char wave[4];                // 'WAVE'
    char fmtChunkMarker[4];      // 'fmt '
    unsigned int lengthOfFmt;    // 포맷 청크의 길이
    unsigned short formatType;   // 포맷 타입
    unsigned short channels;     // 채널 수
    unsigned int sampleRate;     // 샘플 레이트
    unsigned int byteRate;       // (샘플 레이트 * 비트 깊이 * 채널) / 8
    unsigned short blockAlign;   // (비트 깊이 * 채널) / 8
    unsigned short bitsPerSample;// 비트 깊이
};

void playWavFile(const std::string &filename) {
    std::ifstream wavFile(filename, std::ios::binary);
    WavHeader wavHeader;

    if (wavFile.is_open()) {
        wavFile.read((char*)&wavHeader, sizeof(WavHeader));
    } else {
        ROS_ERROR("Could not open WAV file");
        return;
    }

    snd_pcm_t *pcm_handle;
    snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, wavHeader.channels, wavHeader.sampleRate, 1, 500000);

    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    while (!wavFile.eof()) {
        wavFile.read(buffer, BUFFER_SIZE);
        snd_pcm_writei(pcm_handle, buffer, BUFFER_SIZE / (wavHeader.bitsPerSample / 8));
    }

    wavFile.close();
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
}

int gtCount = 0;
int rtCount = 0;
int noneCount = 0;


void yoloCallback(const std_msgs::String::ConstPtr& msg)
{   
    if(flag){
        return;
    }

    if(cs_flag){
        std::cout << "@@@@@@@@@@@@@@@@@@ cs_flag = true ::::: " << sf_count << '\n';
        SF = SF_list[sf_count];
        thread th(changeSoundThreadFunction);
        th.detach();
        sf_count++;
        if(sf_count == 10){
            std::cout << "@@@@@@@@@@@@@@@@@@ sf_count == 10\n";
            sf_count = 10;
            cs_flag = false;
        }
        return;
    }

    

    // if(!flag && soundFlag == true){
    //     playDLSound();
    //     soundFlag = false;
    //     std::cout << "ROS to DL\n";
    // }
    // std::thread input_thread(toggleFlag);
    // ROS_INFO("Received yolo message: %s", msg->data.c_str());
    
    std::cout << msg -> data.c_str() << "@@@@@@" << endl;
    string msg_str = msg -> data.c_str();
    if(msg_str.compare("GT") == 0){
        gtCount++;

        SF = 300;
        thread th(changeSoundThreadFunction);
        th.detach();
    }
    if(msg_str.compare("RT") == 0){
        rtCount++;
    }
    if(msg_str.compare("NONE") == 0){
        noneCount++;
    }
    
    std::cout << gtCount <<std::endl;
    std::cout << rtCount <<std::endl;
    std::cout << noneCount <<std::endl;
}

int main(int argc, char **argv)
{   
    std::thread input_thread(toggleFlag);
    

    ros::init(argc, argv, "pointcloud2_subscriber");
    ros::NodeHandle nh;
    
    ros::Subscriber scan_sub = nh.subscribe<sensor_msgs::PointCloud2>("scan_3D", 5, scanCallback);
    ros::Subscriber yolo_sub = nh.subscribe<std_msgs::String>("chatter", 1, yoloCallback);
    
    // ros::spin();
    while (ros::ok()){
        if (global_flag.load()) {
            // Flag가 true인 경우
            // std::cout << "Flag is true." << std::endl;
            flag = true;

            // 이곳에서 원하는 작업을 수행합니다.
        } else {
            // Flag가 false인 경우
            flag = false;
        }

        ros::spinOnce();
    }

    
    return 0;
    


}
