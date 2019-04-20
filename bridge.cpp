
//TCP/IP
//http://www.cnblogs.com/zkfopen/p/9441264.html
//https://blog.csdn.net/u012234115/article/details/54142273
#include<stdio.h>
#include<stdlib.h>
//#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
//ROS
#include "ros/ros.h"
#include "std_msgs/Float64.h"

#include <string>
#include<sstream>
#include<iostream>
#include <vector>

#define MAXLINE 4096
#define SEVER_PORT 5099
using namespace std;
int main(int argc, char** argv){ 
    
    ros::init(argc, argv, "windows");//初始化ROS，指定节点名字
    ros::NodeHandle nh;//句柄
    ros::Publisher humanPose_ls0 = nh.advertise<std_msgs::Float64>("humanPose/Left/s0", 1000);
    ros::Publisher humanPose_ls1 = nh.advertise<std_msgs::Float64>("humanPose/Left/s1", 1000);
    ros::Publisher humanPose_le0 = nh.advertise<std_msgs::Float64>("humanPose/Left/e0", 1000);
    ros::Publisher humanPose_le1 = nh.advertise<std_msgs::Float64>("humanPose/Left/e1", 1000);
    ros::Publisher humanPose_lw1 = nh.advertise<std_msgs::Float64>("humanPose/Left/w1", 1000);

    ros::Publisher humanPose_rs0 = nh.advertise<std_msgs::Float64>("humanPose/Right/s0", 1000);
    ros::Publisher humanPose_rs1 = nh.advertise<std_msgs::Float64>("humanPose/Right/s1", 1000);
    ros::Publisher humanPose_re0 = nh.advertise<std_msgs::Float64>("humanPose/Right/e0", 1000);
    ros::Publisher humanPose_re1 = nh.advertise<std_msgs::Float64>("humanPose/Right/e1", 1000);
    ros::Publisher humanPose_rw1 = nh.advertise<std_msgs::Float64>("humanPose/Right/w1", 1000);
    ros::Rate loop_rate(10);
    int count = 0;
    cout<<"Initiallizing network..."<<endl;

    int listenfd, connfd;
    struct sockaddr_in  servaddr;
    char buff[4096];
    int n;
 
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    memset(&servaddr, 0, sizeof(servaddr)); //置零
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SEVER_PORT);
    if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        //printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    if( listen(listenfd, 10) == -1){
        //printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    //printf("======waiting for client's request======\n");
    while(ros::ok()){
        if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
            //printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
            continue;
        }else{
            //printf("accept client %s \n",inet_ntoa(servaddr.sin_addr));
	        send(connfd,"Welcome to ubuntu server\n",25,0);//发送欢迎信息
        }
        while((n = recv(connfd,buff,MAXLINE,0))>0&&ros::ok()){
            buff[n] = '\0';
            printf("msg from client: %s\n",buff);

            std_msgs::Float64 angle_ls0;
            std_msgs::Float64 angle_ls1;
            std_msgs::Float64 angle_le0;
            std_msgs::Float64 angle_le1;
            std_msgs::Float64 angle_lw1;
            std_msgs::Float64 angle_rs0;
            std_msgs::Float64 angle_rs1;
            std_msgs::Float64 angle_re0;
            std_msgs::Float64 angle_re1;
            std_msgs::Float64 angle_rw1;

            vector<double> angles;
            char *p;
            char *split = ",";
            p = strtok(buff,split);
		    angles.clear();            //Clear array
            char* pEnd;
            int length = 1;

			angles.push_back(strtod(p,&pEnd));
            while(p = strtok(NULL,split)){
                angles.push_back(strtod(p,&pEnd));
                length++;
            }
            if(length<10)cout<<"can't pub, num is less than 10!"<<endl;
            else{
                humanPose_ls0.publish(angles[0]);
                humanPose_ls1.publish(angles[1]);
                humanPose_le0.publish(angles[2]);
                humanPose_le1.publish(angles[3]);
                humanPose_lw1.publish(angles[4]);
                humanPose_rs0.publish(angles[5]);
                humanPose_rs1.publish(angles[6]);
                humanPose_re0.publish(angles[7]);
                humanPose_re1.publish(angles[8]);
                humanPose_rw1.publish(angles[9]);
            }

            //ROS_INFO("%s", ss.str);//和printf有点像 .c_str()

            ros::spinOnce();
            loop_rate.sleep();
            ++count;
        }
        close(connfd);
    }
    close(listenfd);    

    return 0;
}
