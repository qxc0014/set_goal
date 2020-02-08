#include "ros/ros.h"
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#define BUF_SIZE 200
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

int main(int argc, char *argv[])
{
    int i;
    ros::init(argc, argv, "server1");
    ros::NodeHandle n;
    /*创建客户端*/
    MoveBaseClient ac("move_base", true);
    /*创建套接字(IP地址类型、传输方式、传输协议）*/
    int serv_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	struct sockaddr_in serv_addr;
	    
    /*POSIX规范*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(1235);//主机字节序转换为网络字节序16位(因为端口号是16位的)

    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(serv_sock, 20);
    	
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    char buffer[BUF_SIZE] = {0};  //缓冲区
    while (ros::ok())
    {       
    	int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
        int strLen = recv(clnt_sock, buffer, BUF_SIZE, 0);  //接收客户端发来的数据
    	printf("%s",buffer);
        /*提取字符串中的坐标*/
        std::string goal_xy = buffer;
        std::string goal_x,goal_y;
        size_t a = goal_xy.find_first_of('(');
        size_t b = goal_xy.find_first_of(',');
        size_t c = goal_xy.find_first_of(')');
        goal_x = goal_xy.substr(a+1,b-a-1);//第一个参数是起始位置，第二个参数是指定长度
        goal_y = goal_xy.substr(b+1,c-b-1);
        std::cout << "goal_x:" << goal_x << "goal_y:" <<goal_y << std::endl;
             /*等待服务端响应*/
             while(!ac.waitForServer(ros::Duration(5.0))){
	std::cout << "正在等待服务器回应" << std::endl;             
}
             /*创建目标对象*/
             move_base_msgs::MoveBaseGoal goal;
             /*填充目标对象*/
             goal.target_pose.header.frame_id = "map";
             goal.target_pose.header.stamp = ros::Time::now();
             goal.target_pose.pose.position.x = atof(goal_x.c_str());
             goal.target_pose.pose.position.y = atof(goal_y.c_str());
             goal.target_pose.pose.orientation.w = 1.0;
             std::cout << "发送目标点,正在前往目标" << std::endl;
             /*客户端发送目标点*/
             ac.sendGoal(goal);
             ac.waitForResult();
             if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
                std::cout << "到达目标点!" << std::endl;
             else
                ROS_INFO("The base failed to achieve");


        send(clnt_sock, buffer, strLen, 0);  //将数据原样返回
        close(clnt_sock);
        memset(buffer, 0, BUF_SIZE);  //重置缓冲区
    }
    close(serv_sock);
    return 0;
}
