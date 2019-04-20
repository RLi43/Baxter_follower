// skeleton.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// Remember to Change the IP Address !!!


#include "pch.h"
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
//网络连接
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <stdio.h>
#define SERVER_PORT 5099 //Keep it same with the program running on Ubuntu
#define MAX_BUFF_SIZE 4096
// OpenCV 头文件
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
// Kinect for Windows SDK 头文件
#include <Kinect.h>
constexpr auto ALPHA = 0.35;
constexpr auto EPXLON = 0.015;
constexpr auto SLEEP_TIME = 50;
constexpr auto PI = 3.1415926535897932384626;

float joint[8][3] = { 0 };//全局变量



using namespace std;
using namespace cv;
#pragma comment(lib, "ws2_32.lib")


const string  get_name(int n);    //此函数判断出关节点的名字
void DrawLine(Mat& Img, const Joint& r1, const Joint& r2, ICoordinateMapper* pMapper);


void getmag(Joint J1, Joint J2, float (&mag)[3])
{
	mag[0] = J2.Position.X - J1.Position.X;
	mag[1] = J2.Position.Y - J1.Position.Y;
	mag[2] = J2.Position.Z - J1.Position.Z;
}

//点乘
float dotpro(float mag1[], float mag2[])
{
	return mag1[0] * mag2[0] + mag1[1] * mag2[1] + mag1[2] * mag2[2];
}

//叉乘
void mulcro(float mag1[], float mag2[], float (&mag3)[3])
{
	mag3[0] = mag1[1] * mag2[2] - mag1[2] * mag2[1];
	mag3[1] = mag1[2] * mag2[0] - mag1[0] * mag2[2];
	mag3[2] = mag1[0] * mag2[1] - mag1[1] * mag2[0];
}

//返回两向量夹角
float GetVecAng(float mag1[], float mag2[])
{
	return acos(dotpro(mag1, mag2) / sqrt(dotpro(mag1, mag1)*dotpro(mag2, mag2)));
}
void realupdate(float s0_L, float s1_L, float e0_L, float e1_L, float w1_L, float s0_R, float s1_R, float e0_R, float e1_R, float w1_R)
{
	s0_L = s0_L > PI / 2 ? PI / 2 : s0_L;
	s0_L = s0_L < -PI / 2 ? -PI / 2 : s0_L;
	s1_L = s1_L > PI / 3 ? PI / 3 : s1_L;
	s1_L = s1_L < -2 * PI / 3 ? -2 * PI / 3 : s1_L;
	e0_L = e0_L > 5 * PI / 6 ? 5 * PI / 6 : e0_L;
	e0_L = e0_L < -5 * PI / 6 ? -5 * PI / 6 : e0_L;
	e1_L = e1_L > 5 * PI / 6 ? 5 * PI / 6 : e1_L;
	e1_L = e1_L < 0 ? 0 : e1_L;
	w1_L = w1_L > 2 * PI / 3 ? 2 * PI / 3 : w1_L;
	w1_L = w1_L < -PI / 2 ? -PI / 2 : w1_L;

	s0_R = s0_R > PI / 2 ? PI / 2 : s0_R;
	s0_R = s0_R < -PI / 2 ? -PI / 2 : s0_R;
	s1_R = s1_R > PI / 3 ? PI / 3 : s1_R;
	s1_R = s1_R < -2 * PI / 3 ? -2 * PI / 3 : s1_R;
	e0_R = e0_R > 5 * PI / 6 ? 5 * PI / 6 : e0_R;
	e0_R = e0_R < -5 * PI / 6 ? -5 * PI / 6 : e0_R;
	e1_R = e1_R > 5 * PI / 6 ? 5 * PI / 6 : e1_R;
	e1_R = e1_R < 0 ? 0 : e1_R;
	w1_R = w1_R > 2 * PI / 3 ? 2 * PI / 3 : w1_R;
	w1_R = w1_R < -PI / 2 ? -PI / 2 : w1_R;
}

void update(Joint J[])
{
	for (int i = 0; i < 8; i++)
	{
		if (abs(joint[i][0] - J[i + 4].Position.X) > EPXLON)
			joint[i][0] = ALPHA * J[i + 4].Position.X + (1 - ALPHA)*joint[i][0];
		if (abs(joint[i][1] - J[i + 4].Position.Y) > EPXLON)
			joint[i][1] = ALPHA * J[i + 4].Position.Y + (1 - ALPHA)*joint[i][1];
		if (abs(joint[i][2] - J[i + 4].Position.Z) > EPXLON)
			joint[i][2] = ALPHA * J[i + 4].Position.Z + (1 - ALPHA)*joint[i][2];
	}
}
void getmag(float  J1[], float J2[], float(&mag)[3])
{
	mag[0] = J2[0] - J1[0];
	mag[1] = J2[1] - J1[1];
	mag[2] = J2[2] - J1[2];
}

int main()
{
	//--------TCP/IP服务---------------
	char sever_IP[20] = "192.168.0.7";//
	//cout << "目标IP";
	//cin >> sever_IP;

	//加载套接字
	WSADATA wsaData;
	char buff[MAX_BUFF_SIZE];
	memset(buff, 0, sizeof(buff));
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout<<"Failed to load Winsock"<<endl;
		return 0;
	}
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(SERVER_PORT);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(sever_IP);//127.0.0.1
	//创建套接字
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == sockClient) {
		cout << "Socket() error:" << WSAGetLastError() << endl;
		return 0;
	}
	//向服务器发出连接请求
	if (connect(sockClient, (struct  sockaddr*)&addrSrv, sizeof(addrSrv)) == INVALID_SOCKET) {
		cout << "Connect failed:"<< WSAGetLastError()<<endl;
		return 0;
	}
	char buffSend[MAX_BUFF_SIZE];	

	//发送数据
	/*cout << "q to quit" << endl;
	while (1) {
		cin.getline(buffSend, 100);
		if (buffSend[0] == 'q')break;
		send(sockClient, buffSend, strlen(buffSend) + 1, 0);
		cout << "send!"<<buffSend << endl;
	}*/

	//--------End of TCP/IP服务---------------*/

	/*--- Kinect ----*/
	//
	// 1a. 获取传感器
	IKinectSensor* pSensor = nullptr;
	GetDefaultKinectSensor(&pSensor);

	// 1b. 打开传感器
	pSensor->Open();

	//******************* 2. 彩色图像读取到图像矩阵中******************
	IColorFrameSource* pFrameSource = nullptr;
	pSensor->get_ColorFrameSource(&pFrameSource);
	int     iWidth = 0, iHeight = 0;
	IFrameDescription* pFrameDescription = nullptr;
	pFrameSource->get_FrameDescription(&pFrameDescription);
	pFrameDescription->get_Width(&iWidth);
	pFrameDescription->get_Height(&iHeight);
	IColorFrameReader* pColorFrameReader = nullptr;
	pFrameSource->OpenReader(&pColorFrameReader);

	pFrameDescription->Release();
	pFrameDescription = nullptr;
	pFrameSource->Release();
	pFrameSource = nullptr;

	// Prepare OpenCV data
	UINT uBufferSize = 0;
	Mat mColorImg(iHeight, iWidth, CV_8UC4);
	uBufferSize = iHeight * iWidth * 4 * sizeof(BYTE);


	// *******************3. 读取关节数据************************
	IBodyFrameReader* pBodyFrameReader = nullptr;
	IBody** aBodyData = nullptr;
	INT32 iBodyCount = 0;

	IBodyFrameSource* pBodySource = nullptr;
	pSensor->get_BodyFrameSource(&pBodySource);
	pBodySource->get_BodyCount(&iBodyCount);

	aBodyData = new IBody*[iBodyCount];
	for (int i = 0; i < iBodyCount; ++i)
		aBodyData[i] = nullptr;

	pBodySource->OpenReader(&pBodyFrameReader);
	pBodySource->Release();
	pBodySource = nullptr;

	// *************************4.准备坐标转换*************************
	ICoordinateMapper* pCoordinateMapper = nullptr;
	pSensor->get_CoordinateMapper(&pCoordinateMapper);
	namedWindow("Body Image");
	namedWindow("colorImage");
	time_t time_old = clock();
	while (1)
	{
		time_t time_now = clock();
		if (difftime(time_now,time_old) < SLEEP_TIME)continue;
		time_old = time_now;

		// 4a. 读取彩色图像并输出到矩阵
		IColorFrame* pColorFrame = nullptr;
		if (pColorFrameReader->AcquireLatestFrame(&pColorFrame) == S_OK)
		{
			pColorFrame->CopyConvertedFrameDataToArray(uBufferSize, mColorImg.data, ColorImageFormat_Bgra);
			pColorFrame->Release();
		}
		//Mat mImg = mColorImg.clone();
		Mat mImg(iHeight, iWidth, CV_8UC4);;
		// 4b. 读取Body数据并输出到数组
		IBodyFrame* pBodyFrame = nullptr;
		if (pBodyFrameReader->AcquireLatestFrame(&pBodyFrame) == S_OK)
		{
			// 4b1. 获取身体数据
			if (pBodyFrame->GetAndRefreshBodyData(iBodyCount, aBodyData) == S_OK)
			{
				// 4b2. 遍历每个人
				for (int i = 0; i < iBodyCount; ++i)
				{
					IBody* pBody = aBodyData[i];

					// 4b3. 确认追踪状态
					BOOLEAN bTracked = false;
					if ((pBody->get_IsTracked(&bTracked) == S_OK) && bTracked)
					{
						// 4b4.获取关节
						Joint aJoints[JointType::JointType_Count];
						if (pBody->GetJoints(JointType::JointType_Count, aJoints) == S_OK)
						{
							DrawLine(mImg, aJoints[JointType_SpineBase], aJoints[JointType_SpineMid], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_SpineMid], aJoints[JointType_SpineShoulder], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_SpineShoulder], aJoints[JointType_Neck], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_Neck], aJoints[JointType_Head], pCoordinateMapper);

							DrawLine(mImg, aJoints[JointType_SpineShoulder], aJoints[JointType_ShoulderLeft], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_ShoulderLeft], aJoints[JointType_ElbowLeft], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_ElbowLeft], aJoints[JointType_WristLeft], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_WristLeft], aJoints[JointType_HandLeft], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_HandLeft], aJoints[JointType_HandTipLeft], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_HandLeft], aJoints[JointType_ThumbLeft], pCoordinateMapper);

							DrawLine(mImg, aJoints[JointType_SpineShoulder], aJoints[JointType_ShoulderRight], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_ShoulderRight], aJoints[JointType_ElbowRight], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_ElbowRight], aJoints[JointType_WristRight], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_WristRight], aJoints[JointType_HandRight], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_HandRight], aJoints[JointType_HandTipRight], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_HandRight], aJoints[JointType_ThumbRight], pCoordinateMapper);

							DrawLine(mImg, aJoints[JointType_SpineBase], aJoints[JointType_HipLeft], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_HipLeft], aJoints[JointType_KneeLeft], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_KneeLeft], aJoints[JointType_AnkleLeft], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_AnkleLeft], aJoints[JointType_FootLeft], pCoordinateMapper);

							DrawLine(mImg, aJoints[JointType_SpineBase], aJoints[JointType_HipRight], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_HipRight], aJoints[JointType_KneeRight], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_KneeRight], aJoints[JointType_AnkleRight], pCoordinateMapper);
							DrawLine(mImg, aJoints[JointType_AnkleRight], aJoints[JointType_FootRight], pCoordinateMapper);

							//----关节角输出与映射---------
							//----mapping ---------------

							JointOrientation aOrientations[JointType::JointType_Count];
							if (pBody->GetJointOrientations(JointType::JointType_Count, aOrientations) == S_OK)
							{
								string str;
								stringstream ss;
								//Joint information Left Limb and Right Limb
								/*for (int i = 4; i <12; i++) {
									cout << "J" << i << "(" << get_name(i) << ")" << endl;
									cout << " pos[" << aJoints[i].Position.X << "," << aJoints[i].Position.Y << "," << aJoints[i].Position.Z << "]" << endl;
									cout << " ori[" << aOrientations[i].Orientation.w<<"," << aOrientations[i].Orientation.x << "," << aOrientations[i].Orientation.y << "," << aOrientations[i].Orientation.z <<"]"<< endl;
									cout << endl;
								}*/

								//mapping
								update(aJoints);
								
								float m45[3], m56[3], m67[3], m89[3], m90[3], m01[3], n1[3], n2[3];
								float z[3] = { 0,1,0 };
								float s0_L, s1_L, e0_L, e1_L, w1_L, s0_R, s1_R, e0_R, e1_R, w1_R;
								getmag(joint[0], joint[1], m45);
								getmag(joint[1], joint[2], m56);
								getmag(joint[2], joint[3], m67);
								getmag(joint[4], joint[5], m89);
								getmag(joint[5], joint[6], m90);
								getmag(joint[6], joint[7], m01);

								/*float m45[3], m56[3], m67[3], m89[3], m90[3], m01[3], n1[3], n2[3];
								float z[3] = { 0,1,0 };
								float s0_L, s1_L, e0_L, e1_L, w1_L, s0_R, s1_R, e0_R, e1_R, w1_R;
								getmag(aJoints[4], aJoints[5], m45);
								getmag(aJoints[5], aJoints[6], m56);
								getmag(aJoints[6], aJoints[7], m67);
								getmag(aJoints[8], aJoints[9],m89);
								getmag(aJoints[9], aJoints[10], m90);
								getmag(aJoints[10], aJoints[11], m01);*/

								//left
								mulcro(m45, z, n1);//n1为m45与竖直平面的法向量
								mulcro(m45, m56, n2);//n2为m45，m56叉乘结果(456平面法向量)
								s0_L = -atan(m45[2] / m45[0])+PI/6;// +PI / 4;
								s1_L = PI/2 - acos(-m45[1] / sqrt(dotpro(m45, m45)));
								e0_L = - PI + GetVecAng(n1, n2);
								e1_L = GetVecAng(m45, m56);
								w1_L = GetVecAng(m56, m67);

								//right
								mulcro(m89, z, n1);//n1为m45与竖直平面的法向量
								mulcro(m89, m90, n2);//n2为m45，m56叉乘结果(456平面法向量)
								s0_R = atan(-m89[2] / m89[0])-PI/6;// -PI / 4;
								s1_R = PI/2 - acos(-m89[1] / sqrt(dotpro(m89, m89)));
								e0_R = PI - GetVecAng(n1, n2);
								e1_R = GetVecAng(m89, m90);
								w1_R = GetVecAng(m90, m01);
								realupdate(s0_L, s1_L, e0_L, e1_L, w1_L, s0_R, s1_R, e0_R, e1_R, w1_R);


								ss << s0_L << "," << s1_L << "," << e0_L << "," << e1_L << "," << w1_L<<",";
								ss << s0_R << "," << s1_R << "," << e0_R << "," << e1_R << "," << w1_R << endl;
								str = ss.str();
								str.copy(buffSend, str.length(), 0);
								buffSend[str.length() + 1] = '\0';
								//cout << "char" << buffSend << endl;
								send(sockClient, buffSend, strlen(buffSend) + 1, 0);
								//debug
								//float deg = 180 / 3.14159;	
								//for (int i = 0; i < 8; i++) {
								//	cout <<i<<": "<<joint[i][0]<< "," << joint[i][1] <<"," << joint[i][2] << endl;
								//}
								//cout << endl;
								//cout <<"L "<<s0_L*deg << "," << s1_L*deg << "," << e0_L*deg << ","<< e1_L*deg <<","<<w1_L*deg<< endl;

								//cout <<"R "<< s0_R * deg << "," << s1_R * deg << "," << e0_R * deg << "," << e1_R * deg << "," << w1_R * deg <<endl;

								//Sleep(SLEEP_TIME);
							}
						}
					}
				}
			}
			else
			{
				cerr << "Can't read body data" << endl;
			}

			// 4e. 释放bodyframe
			pBodyFrame->Release();
		}

		// 输出图像
		imshow("Body Image", mImg);
		imshow("colorImage", mColorImg);

		if (waitKey(30) == VK_ESCAPE) {
			break;
		}
	}
	delete[] aBodyData;

	// 3.释放frame reader
	cout << "Release body frame reader" << endl;
	pBodyFrameReader->Release();
	pBodyFrameReader = nullptr;

	// 2. 释放 color frame reader
	cout << "Release color frame reader" << endl;
	pColorFrameReader->Release();
	pColorFrameReader = nullptr;

	// 1c.关闭Sensor
	cout << "close sensor" << endl;
	pSensor->Close();

	// 1d. 释放Sensor
	cout << "Release sensor" << endl;
	pSensor->Release();
	pSensor = nullptr;
	//

	//关闭套接字
	closesocket(sockClient);
	WSACleanup();

	return 0;

}
const   string  get_name(int n)
{
	switch (n)
	{
	case 0:return "Spine Base"; break;
	case 1:return "Spine Mid"; break;
	case 2:return "Neck"; break;
	case 3:return "Head"; break;

	case 4:return "Left Shoulder"; break;
	case 5:return "Left Elbow"; break;
	case 6:return "Left Wrist"; break;
	case 7:return "Left Hand"; break;

	case 8:return "Right Shoulder"; break;
	case 9:return "Right Elbow"; break;
	case 10:return "Right Wrist"; break;
	case 11:return "Right Hand"; break;

	case 12:return "Left Hip"; break;
	case 13:return "Left Knee"; break;
	case 14:return "Left Ankle"; break;
	case 15:return "Left Foot"; break;

	case 16:return "Right Hip"; break;
	case 17:return "Right Knee"; break;
	case 18:return "Right Ankle"; break;
	case 19:return "Right Foot"; break;

	case 20:return "Spine Shoulder"; break;
	case 21:return "Left HandTip"; break;
	case 22:return "Left Thumb"; break;
	case 23:return "Right HandTip"; break;
	case 24:return "Right Thumb"; break;
	default:return "NULL";
	}
}
void DrawLine(Mat& Img, const Joint& r1, const Joint& r2, ICoordinateMapper* pMapper)
{
	//用两个关节点来做线段的两端，并且进行状态过滤
	if (r1.TrackingState == TrackingState_NotTracked || r2.TrackingState == TrackingState_NotTracked)
		return;
	//要把关节点用的摄像机坐标下的点转换成彩色空间的点
	ColorSpacePoint p1, p2;
	pMapper->MapCameraPointToColorSpace(r1.Position, &p1);
	pMapper->MapCameraPointToColorSpace(r2.Position, &p2);

	line(Img, Point(p1.X, p1.Y), Point(p2.X, p2.Y), Vec3b(0, 0, 255), 5);
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
