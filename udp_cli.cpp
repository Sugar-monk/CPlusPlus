/*************************************************************************
-> 文件名: udp_cli.cpp
-> 作者: bean
-> 邮箱: 17331372728@163.com
-> 创建时间: Fri 24 May 2019 05:06:34 AM PDT
-> 备注: 通过udpsocket实现客户端程序
*************************************************************************/
#include "udp_socket.hpp"

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout << "./dup_cli ip port" << endl;
        return -1;
    }
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    UdpSocket sock;
    CHECK_RET(sock.Socket());

    struct sockaddr_in s_addr;
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    s_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    while(1)
    {

        string buff;
        cout << "client say:";
        getline(cin, buff);
        sock.Send(buff, s_addr);

        buff.clear();
        sock.Recv(buff);
        cout << "server say:" << buff << endl;

    }

    sock.Close();
	return 0;
}

