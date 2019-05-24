/*************************************************************************
-> 文件名: udp_ser.cpp
-> 作者: bean
-> 邮箱: 17331372728@163.com
-> 创建时间: Thu 23 May 2019 04:08:16 AM PDT
-> 备注:使用UdpSock类完胜UDP服务端程序
*************************************************************************/
#include"udp_socket.hpp"

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        cout << "./dup_srv ip port" << endl;
        return -1;
    }
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);
    UdpSocket sock;
    //创建套接字
    CHECK_RET(sock.Socket());
    CHECK_RET(sock.Bind(ip, port));


    while(1)
    {
        string buff;
        struct sockaddr_in c_addr;
        sock.Recv(buff, &c_addr);
        cout << "client asy:" << buff.c_str() << endl;

        cout << "server say:";
        fflush(stdout);
        getline(cin, buff);
        sock.Send(buff, c_addr);
    }

    sock.Close();

	return 0;
}

