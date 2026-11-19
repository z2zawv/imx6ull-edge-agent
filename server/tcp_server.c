/*
 TCP服务端程序：
1.创建一个 TCP Socket。
2.监听本机的 8888 端口。
3.等待客户端连接。
4.接收客户端发送的数据，并返回 ACK。
5.客户端断开后，继续等待下一个客户端。（nc模拟客户端）
    核心概念：服务端不是主动找客户端，而是在某个端口等待客户端来连接

流程：创建监听Socket[socket()]--->绑定IP和端口[bind()]--->开始监听[listen()]--->等待客户端连接[accept()]--->
     接收数据并回复ACK[recv()-->send()]--->关闭客户端连接[close(client_id)]--->返回accept()，等待下一次连接
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main()
{
    int server_fd;  // 负责监听新的TCP连接
    int client_fd;  //负责已经建立通信的TCP连接
    int opt=1;


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

    printf("TCP Server listening on port 8888...\n");
   
   
   
    //等待客户端连接,建立连接后返回client_fd
    while(1)                            //无限循环，让服务器能够不断接收客户端连接
    {
        client_len=sizeof(client_addr);

        client_fd=accept(server_fd,(struct sockaddr *)&client_addr,&client_len);
        if(client_fd<0)
        {
            perror("accpet");
            break;             //返回while循环等待连接
        }

        inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,sizeof(client_ip));
        printf("client ip:%s,%d\n",client_ip,ntohs(client_addr.sin_port));


        //接收客户端数据
        while(1)
        {
            ssize_t n=recv(client_fd,buf,sizeof(buf),0);    //ssize_t 是能够表达非负字节数和负错误值的有符号整数类型
            if(n>0)
            {
                printf("buf capacity:%zu\n",sizeof(buf));
                printf("received %ld bytes:",(long)n);
                fwrite(buf, 1, n, stdout);
                  printf("\n");

                const char ACK[]="ACK: message received\n";     //ACK 在这里可以理解为应用程序自定义的确认文字
                if(send(client_fd,ACK,sizeof(ACK)-1,0)<0)
                {
                    perror("send");
                    break;
                }
            }

            else if (n==0)
            {
                 printf("Client disconnected.\n");
                 break;
            }

            else
            {
                 perror("recv");
                break;
            }
           
           
        }

         close(client_fd);
    }





close(server_fd);


return EXIT_SUCCESS;

}