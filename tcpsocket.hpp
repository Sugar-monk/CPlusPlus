/*************************************************************************
-> 文件名: tcpsocket.hpp
-> 作者: bean
-> 邮箱: 17331372728@163.com
-> 创建时间: Fri 24 May 2019 06:05:41 AM PDT
-> 备注: 封装一个TcpSocket类，向外提供轻便的socket接口
        1.创建套接字
        2.为套接字绑定地址
        3.客户端向服务端发起连接请求
        4.服务端开始监听
        4.服务端获取一个已经连接成功的客户端的新建socket
        5.客户端与服务端开始收发数据
        6.关闭套接字
*************************************************************************/
#include<iostream>
#include<string>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
using namespace std;

#define CHECK_RET(q) if((q) == false){return -1;}

class TcpSocket
{
    public:
       TcpSocket():_sockfd(-1)
         {}
       ~TcpSocket(){}
       bool Socket()
       {
           _sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
           if(_sockfd < 0)
           {
               cout << "socket error" << endl;
               return false;
           }
           return true;
       }
       bool Bind(string &ip, uint16_t port)
       {
           struct sockaddr_in addr;
           addr.sin_family = AF_INET;
           addr.sin_port = htons(port);
           addr.sin_addr.s_addr = inet_addr(ip.c_str());
           socklen_t len = sizeof(struct sockaddr_in);

           int ret = bind(_sockfd, (struct sockaddr*)&ip, len);
           if(ret < 0)
           {
               cout << "bind error" << endl;
               return false;
           }
       }
       //开始监听，并且设置服务端的同一时间最大并发连接数
       bool Listen(int baklog = 5)
       {
           //int listen(int sockfd, int backlog);
           //sockfd:    套接字描述符
           //backlog:   同一时间最大连接数，设置内核中已完成连接的最大节点数
           int ret = listen(_sockfd, baklog);
           if(ret < 0)
           {
               cout << "Listen error" << endl;
               return false;
           }
           return true;
       }
       //获取连接成功客户端socket，并且返回客户端的地址信息
       bool Accept(TcpSocket &sock, struct sockaddr_in *addr = NULL)
       {
           //int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
           //sockfd:    套接字描述符
           //addr:      用于存储客户端地址信息
           //addrlen:   用于设置想要的地址长度和保存实际的地址长度
           //返回值：   为客户端连接新建的socket描述符
           //接下来与客户端的通信都是通过这个新的描述符实现的
           struct sockaddr_in _addr;
           socklen_t len = sizeof(struct sockaddr_in);
           int newfd = accept(_sockfd, (struct sockaddr*)&_addr, &len);
           if(newfd < 0)
           {
               cout << "Accept error" << endl;
               return false;
           }
           sock.Setfd(newfd);
           if(addr != NULL)
           {
               memcpy(addr, &_addr, len);
           }
           return true;
       }
       //客户端向服务端发起连接请求
       bool Connect(string &ip, uint16_t port)
       {
           //int connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
           //sockfd:        套接字描述符
           //addr:          服务端监听地址信息
           //addrlen:       地址信息长度
           struct sockaddr_in addr;
           addr.sin_family = AF_INET;
           addr.sin_port = htons(port);
           addr.sin_addr.s_addr = inet_addr(ip.c_str());
           socklen_t len = sizeof(struct sockaddr_in);

           int ret = connect(_sockfd,(struct sockaddr*)&addr, len);
           if(ret < 0)
           {
               cout << "connect error" << endl;
               return false;
           } 
           return true;
       }
       //通信接口：tcp服务端也可以直接发送数据（因为连接已经发送成功）
       bool Recv(string &buff)
       {
           //ssize_t recv(int sockfd, void *buf, size_t len, int flags);
           //sockfd:        套接字描述符
           //buf:           接收的数据存储
           //len:           想要接收的数据长度
           //flags:         0 默认是阻塞接收   
           //               MSG_PEEK 接收数据但是数据不从接收缓冲区中移除   探测性接收
           //返回值：       成功返回实际接收字节数，错误返回的-1，对端已经关闭了连接返回0
           char tmp[1024] = {0};
           int ret = recv(_sockfd, tmp, 1024, 0);
           if(ret == 0)
           {
               cout << "peer shutdown" << endl;
               return false;
           }
           else if(ret < 0)
           {
               cout << "recv error" << endl;
               return false;
           }
           buff.assign(tmp,ret);
           return true;
       }
       ssize_t Send(string &buff)
       {
           //ssize_t send(int sockfd, const void *buf, size_t len, int flags);
           //返回值：   成功就是实际发送的字节数    失败 -1
           //如果连接已经断开，发送就会触发异常，导致进程退出
           int ret = send(_sockfd, buff.c_str(), buff.size(), 0);
           if(ret < 0)
           {
               cout << "send error" << endl;
               return false;
           }
           return true;
       }
       bool Close()
       {
           close(_sockfd);
           _sockfd = -1;
           return true;
       }
    private:
       void Setfd(int sockfd)
       {
           _sockfd = sockfd;
       }
    private:
       int _sockfd;
};
