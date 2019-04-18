// skeleton.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//


#include "pch.h"
#include <iostream>
#include <string>
#include <sstream>
//网络连接
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <stdio.h>
#define SERVER_PORT 5099
#define MAX_BUFF_SIZE 1024
// OpenCV 头文件
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
// Kinect for Windows SDK 头文件
#include <Kinect.h>

using namespace std;
using namespace cv;
#pragma comment(lib, "ws2_32.lib")

const   string  get_name(int n);    //此函数判断出关节点的名字
void DrawLine(Mat& Img, const Joint& r1, const Joint& r2, ICoordinateMapper* pMapper);

int main()
{
	//--------TCP/IP服务---------------
	char sever_IP[20] = "192.168.0.20";//
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

	char buffSend[5000];	

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

	while (1)
	{
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

							//

							JointOrientation aOrientations[JointType::JointType_Count];
							if (pBody->GetJointOrientations(JointType::JointType_Count, aOrientations) == S_OK)
							{
								string str;
								stringstream ss;
								for (int i = 4; i <12; i++) {
									cout << "J" << i << "(" << get_name(i) << ")" << endl;
									cout << " pos[" << aJoints[i].Position.X << "," << aJoints[i].Position.Y << "," << aJoints[i].Position.Z << "]" << endl;
									cout << " ori[" << aOrientations[i].Orientation.w<<"," << aOrientations[i].Orientation.x << "," << aOrientations[i].Orientation.y << "," << aOrientations[i].Orientation.z <<"]"<< endl;
									cout << endl;

									//ss << i;
									//ss << i << ":" << aJoints[i].Position.X << "," << aJoints[i].Position.Y << "," << aJoints[i].Position.Z << ":" << aOrientations[i].Orientation.w << "," << aOrientations[i].Orientation.x << "," << aOrientations[i].Orientation.y << "," << aOrientations[i].Orientation.z << endl;
									
									//ss = i+':';
									//ss += aJoints[i].Position.X; ss += ','; ss += aJoints[i].Position.Y; ss += ','; ss += aJoints[i].Position.Z; ss += ':';
									//ss += aOrientations[i].Orientation.w; ss += ','; ss += aOrientations[i].Orientation.x; ss += ','; ss += aOrientations[i].Orientation.y; ss += ','; ss += aOrientations[i].Orientation.z;
									//ss << " pos[" << aJoints[i].Position.X << "," << aJoints[i].Position.Y << "," << aJoints[i].Position.Z << "]" << endl;
									//ss << " ori[" << aOrientations[i].Orientation.w<<"," << aOrientations[i].Orientation.x << "," << aOrientations[i].Orientation.y << "," << aOrientations[i].Orientation.z <<"]"<< endl;
									//ss << endl;
									
									
								}
								float j45[3] = { aJoints[5].Position.X - aJoints[4].Position.X,aJoints[5].Position.Y - aJoints[4].Position.Y,aJoints[5].Position.Z - aJoints[4].Position.Z };
								float j56[3] = { aJoints[6].Position.X - aJoints[5].Position.X,aJoints[6].Position.Y - aJoints[5].Position.Y,aJoints[6].Position.Z - aJoints[5].Position.Z };
								
								float s0_l = -atan(j45[2] / j45[0]) + 0.785;
								float s1_l = 1.57 - acos(-j45[1] / sqrt(j45[0] * j45[0] + j45[1] * j45[1] + j45[2] * j45[2]));
								float e1_l = acos((j45[0] * j56[0] + j45[1] * j56[1] + j45[2] * j56[2]) / (sqrt(j45[0] * j45[0] + j45[1] * j45[1] + j45[2] * j45[2])*sqrt(j56[0] * j56[0] + j56[1] * j56[1] + j56[2] * j56[2])));
								
								float n1[3] = { -j45[2],0,j45[0] };
								float n2[3] = { j45[1] * j56[2] - j45[2] * j56[1], j45[2] * j56[0] - j45[0] * j56[2], j45[0] * j56[1] - j45[1] * j56[0] };
								float e0_l = -3.14 + acos((n1[0] * n2[0] + n1[1] * n2[1] + n1[2] * n2[2]) / (sqrt(n1[0] * n1[0] + n1[1] * n1[1] + n1[2] * n1[2])*sqrt(n2[0] * n2[0] + n2[1] * n2[1] + n2[2] * n2[2])));
											
								float w1_l = 0;
								//float deg = 180 / 3.14159;
								ss << s0_l << "," << s1_l << "," << e0_l << ","<< e1_l <<","<<w1_l<< endl;
								//cout << ss.str()<<endl;
								str = ss.str();
								//cout << "str" << str << endl;
								str.copy(buffSend, str.length(), 0);
								buffSend[str.length() + 1] = '\0';
								cout << "char" << buffSend << endl;
								send(sockClient, buffSend, strlen(buffSend) + 1, 0);
								Sleep(5000);
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
