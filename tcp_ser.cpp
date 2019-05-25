/*************************************************************************
*文件名: tcp_ser.cpp
*作者: bean
*邮箱: 17331372728@163.com
* 创建时间: Fri 24 May 2019 07:27:34 AM PDT
* 备注:通过封装的TcpSocket类实现服务端程序
*       1、创建套接字
*       2、绑定地址信息
*       3、开始监听
*       4、获取连接成功的客户端新建socket
*       5、通过新建的socket与客户端进行通信
*       6、关闭套接字
*************************************************************************/
#include"tcpsocket.hpp"
#include<signal.h>
#include<sys/wait.h>

void sigcb(int no)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout << "./tcp_ser ip port" << endl;
        return -1;
    }
    //如果多个进程同时退出了，要等他处理完了再退出
    signal(SIGCHLD, sigcb);
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);
    TcpSocket sock;

    CHECK_RET(sock.Socket());
    CHECK_RET(sock.Bind(ip, port));
    CHECK_RET(sock.Listen());
    while(1)
    {
        //程序会卡在accept是应为用户无法获知客户端的新连接什么时候到来
        //如果能知道什么时候盗垒就不会阻塞，只需要在来的时候调用一次就行
        TcpSocket client;
        if(sock.Accept(client) == false)
        {
            continue;
        }
        //创建一个子进程负责聊天
        int pid = fork();
        if(pid == 0)
        {
            while(1)
            {
                string buff;
                client.Recv(buff);
                cout << "Client say:" << buff << endl;

                cout << "Server asy:";
                fflush(stdout);
                cin >> buff;
                client.Send(buff);
            }
        }
        else
        {

        }
    }

    sock.Close();

	return 0;
}

