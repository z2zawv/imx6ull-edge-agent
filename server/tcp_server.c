/*
 Day1:TCP服务端程序：
1.创建一个 TCP Socket。
2.监听本机的 8888 端口。
3.等待客户端连接。
4.接收客户端发送的数据，并返回 ACK。
5.客户端断开后，继续等待下一个客户端。（nc模拟客户端）
    核心概念：服务端不是主动找客户端，而是在某个端口等待客户端来连接

流程：创建监听Socket[socket()]--->绑定IP和端口[bind()]--->开始监听[listen()]--->等待客户端连接[accept()]--->
     接收数据并回复ACK[recv()-->send()]--->关闭客户端连接[close(client_id)]--->返回accept()，等待下一次连接

Day2:接受机器人运动指令的控制服务端
2.1建立机器人状态：
               robot_running        int        机器人是否处于运行状态    
               linear_velocity      double     线速度m/s
               angular_velocity     double     加速度rad/s
2.2设置STATUS，STOP，MOVE

*/





#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include "robot_state.h"
#include "command.h"



int main()
{
    int server_fd;  // 负责监听新的TCP连接
    int client_fd;  //负责已经建立通信的TCP连接
    int opt=1;

    //定义状态变量并初始化
    RobotState robot_state;
    robot_state_init(&robot_state);

    struct sockaddr_in server_addr; //保存服务端自己的地址信息
    struct sockaddr_in client_addr; //保存客户端的地址信息

    socklen_t client_len;   //socklen_t 是一种用于表示 Socket 地址长度的数据类型,Socket API 规定地址长度参数使用 socklen_t
    char buf[1024];         //用于接收客户端发送的数据
    char client_ip[INET_ADDRSTRLEN];//保存客户端 IPv4 地址字符串的字符数组;INET_ADDRSTRLEN 是头文件中定义的常量，IPv4 环境下通常是 16


    //创建监听Socket
    server_fd = socket(AF_INET,SOCK_STREAM,0);
    if(server_fd<0)
    {
        perror("socket error");
        return EXIT_FAILURE;//程序以失败状态退出,EXIT_FAILURE 是 stdlib.h 提供的宏.
    }
    //TCP服务端初始化，地址复用
    if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt, sizeof(opt))<0)
    {
        perror("set error");
        close(server_fd);
        return EXIT_FAILURE;

    }


    //服务端配置IP和端口
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(8888);//网络编程中很重要的字节序转换,把主机字节序的 16 位整数转换成网络字节序。
                                    //网络字节序采用大端序，不同 CPU 可能采用不同的主机字节序，所以需要进行转换
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);//INADDR_ANY 表示 IPv4 通配地址，也就是 0.0.0.0。
                                                  //它的作用不是把服务器 IP 真的设置成一台地址为 0.0.0.0 的主机，
                                                  //只要客户端访问的是本机某个有效的 IPv4 地址，并且目标端口是 8888，就可以由这个监听 Socket 接收连接。
    
    //绑定IP和端口：将 Socket 绑定到指定的本地地址
    if(bind(server_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0)//以通用 Socket 地址接口要求的指针类型传递它
    {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    //开始监听
    if(listen(server_fd,5)<0)
    {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }
    //通信系统打印
    printf("TCP Server listening on port 8888...\n");
    //机器人状态打印
    printf("Robot running: %d\n", robot_state.running);
    printf("Linear velocity: %.2f\n", robot_state.linear_velocity);
    printf("Angular velocity: %.2f\n", robot_state.angular_velocity);
   
   
   
    //等待客户端连接,建立连接后返回client_fd
    while(1)                            //无限循环，让服务器能够不断接收客户端连接
    {
        client_len=sizeof(client_addr);
        //client_addr接受客户端的IP和端口号：
        //                               client_addr.sin_addr:IP
        //                               client_addr.sin_port:端口
        client_fd=accept(server_fd,(struct sockaddr *)&client_addr,&client_len);
        if(client_fd<0)
        {
            perror("accpet");
            break;             //退出外层循环
        }
        //client_addr.sin_addr里面存储IP的二进制地址信息，需要 inet_ntop() 将它转换成可读的 IP 地址字符串，将转换结果(字符串)放在client_ip
        //TCP/IP 网络协议中，端口号使用网络字节序（大端序）保存，所以使用ntohs将网络字节序的 16 位端口号转换为主机字节序
        inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,sizeof(client_ip));
        printf("client ip:%s,%d\n",client_ip,ntohs(client_addr.sin_port));

        //服务器目前是一次处理一个客户端连接。每次新客户端连接时，都应该有一个新的、空的接收缓冲区
        char   recv_buf[4096];
        size_t recv_len=0;

        //接收客户端数据
        while(1)
        {
            //安全接收
            ssize_t n=recv(client_fd,buf,sizeof(buf)-1,0);    //ssize_t 是能够表达非负字节数和负错误值的有符号整数类型
            //1.判断接收结果
            if (n==0)
            {
                 printf("Client disconnected.\n");
                 break;
            }

            if(n<0)
            {
                 perror("recv");
                break;
            }
           
            //// 2. 检查缓冲区是否有足够空间
            if((size_t)n>sizeof(recv_buf)-recv_len)
            {
                printf("Receive buffer overflow\n");
                break;
            }
            // 3. 累积接收到的数据并打印
            memcpy(recv_buf+recv_len,buf,(size_t)n);//累积数据
            recv_len +=(size_t)n;
            printf("recv_len = %zu\n", recv_len);
            fwrite(recv_buf, 1, recv_len, stdout);
            printf("\n");

            // 4. 循环提取完整指令
            //识别\n结尾的完整指令并将完整指令取走，将剩余数据移动到缓冲区开头
            //使用 memchr() 查找换行符
            char command[1024];
            int close_client = 0;

            while(1)
            {
                 // 查找换行符
                char *newline = memchr(recv_buf, '\n', recv_len);
                 if(newline == NULL)
                {
                    break;
                }
                 size_t cmd_len = (size_t)(newline - recv_buf);//指令长度

                 //检查指令长度
                if (cmd_len >= sizeof(command))
                {
                    printf("Command too long\n");
                    close_client=1;
                    break;
                }
                // 提取完整指令
                memcpy(command, recv_buf, cmd_len);
                command[cmd_len] = '\0';

                  // 兼容 Windows 客户端的 CRLF 换行
                  //Linux：通常使用 \n 表示换行。
                 //Windows：通常使用 \r\n 表示换行。
                if(cmd_len > 0 && command[cmd_len - 1] == '\r')
                {
                     command[cmd_len - 1] = '\0';
                }

                 printf("Complete command: [%s]\n", command);

                 // 移除已经提取的指令
                size_t consumed=cmd_len+1;
                size_t remaining=recv_len-consumed;
                memmove(recv_buf,recv_buf+consumed,remaining);
                recv_len=remaining;


                // 5. 根据完整指令执行操作
                char response[128];
                 process_command(command, &robot_state,response,sizeof(response));

                // 6. 返回处理结果
                if(send(client_fd, response, strlen(response), 0) < 0)
                {
                            perror("send");
                            close_client = 1;
                            break;
                }
            }

                 if(close_client)
                {
                     break;
                }



                // 如果尚未找到换行符，但未完成数据已经超过指令最大长度
                 if(recv_len >= sizeof(command))
                 {
                    printf("Command too long\n");
                    break;
                }
        }

         close(client_fd);
    }

close(server_fd);


return EXIT_SUCCESS;

}