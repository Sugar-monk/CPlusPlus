/*************************************************************************
-> 文件名: udp_socket.hpp
-> 作者: bean
-> 邮箱: 17331372728@163.com
-> 创建时间: Thu 23 May 2019 04:20:18 AM PDT
-> 备注:分装一个udp的套接字接口类
    实现：
        套接字创建、绑定地址、接收数据、发送数据、关闭套接字
*************************************************************************/

#include<iostream>
#include<string>
#include<sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<sys/types.h>
#include<string.h>

#define CHECK_RET(q) if((q) == false){return -1;}

using namespace std;

class UdpSocket
{
    public:
        UdpSocket()
        {
        }
        ~UdpSocket()
        {
        }
        bool Socket()   //创建套接字
        {
            //int socket(int domain, int type, int protocol);
            //domain:  地址域，确定通信范围
            //      AF_INET     表示使用IPV4网络协议
            //type：    套接字类型
            //      SOCK_STREAM 流式套接字----默认使用TCP
            //      SOCK_DGRSM  数据包套接字--默认使用UDP
            //protocol:
            //      0   使用默认协议
            //      IPPROTO_TCP 6   TCP协议
            //      IPPROTO_UDP 17  UDP协议
            //返回值：套接字操作句柄---文件描述符       失败：-1
            _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(_sock < 0)
            {
                perror("socket error");
                return false;
            }
            return true;
        }
        bool Bind(string &ip, uint16_t port)     //为套接字绑定地址信息
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            //因为网络通信使用大端字节序，因此端口和IP地址信息都要转换成网络字节序
            //uint16_t htons(uint16_t hostshort);
            // 将一个16为数据从主机字节序转换为网络字节序
            //uint32_t htonl(uint32_t hostlong);
            //将一个32为数据从主机字节序转换为网络字节序
            //不能使用htonl，因为端口只有2的字节，由于htonl是两个字节来转换的，所以经htonl后，还是原来的
            addr.sin_port = htons(port);
            //in_addr_t inet_addr(const char *cp);
            //将字符串点分十进制IP地址，转换为网络字节序的整数IP地址
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
            //int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
            //sockfd:       创建套接字返回套接字描述符
            //addr:         要绑定的地址信息
            //addrlen:      地址信息长度
            //返回值：      成功：0     失败：-1
            socklen_t len = sizeof(struct sockaddr_in);
            int ret = bind(_sock, (struct sockaddr*)&addr, len);
            if(ret < 0)
            {
                perror("Bind error");
                return -1;
            }
            return true;
        }
        bool Recv(string &buf, sockaddr_in *_addr = NULL)      //接收数据
        {
            //recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
            //sockfd:       套接字描述符
            //buf:          用于保存接收的数据
            //len:          要接受的数据长度
            //flags:        默认0，表示阻塞接收，没有数据就阻塞
            //src_addr:     发送端的地址信息
            //addelen:      用于获取地址信息长度（输入输出型参数）
            //返回值：      实际接收数据长度，错误返回-1
            char tmp[1024] = {0};
            struct sockaddr_in addr;
            socklen_t len = sizeof(struct sockaddr_in);
            int ret = recvfrom(_sock, tmp, 1024, 0,(struct sockaddr*)&addr, &len);
            if(ret < 0)
            {
                perror("recvfrom error");
                return false;
            }
            buf.assign(tmp, ret);   //从tmp中拷贝获取的实际长度到buf中
            if(_addr != NULL)//如果用户想要获取发送端的地址，这是_addr就不为空，这时就将获取到的地址拷贝到_addr中
                memcpy(_addr,&addr,len);
            return true;
        }
        bool Send(string &buf, struct sockaddr_in &addr)     //发送数据
        {
            //sendto(int sockfd, void *buf, size_t len, int flags,struct sockaddr *dest_addr, socklen_t addrlen);
            //dest_addr:        目的端地址信息
            //addrlen:          发送地址信息长度
            socklen_t len = sizeof(struct sockaddr_in);
            int ret = sendto(_sock, buf.c_str(), buf.size(), (struct sockaddr*)&addr, len);
            if(ret < 0)
            {
                perror("sendto error");
                return false;
            }
            return true;
        }
        bool Close() //关闭套接字
        {
            close(_sock);
            _sock = -1;
            return true;
        }
    private:
        int _sock;
};
