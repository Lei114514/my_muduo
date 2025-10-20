#include <iostream>
#include <thread>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "Acceptor.h"
#include "Logger.h" 
#include "InetAddress.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "TcpConnection.h"
#include "TcpServer.h"

#define IP "192.168.232.137"
#define PORT 8899

using std::cout;
using std::endl;

void echoServer() 
{
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoop* loop=new EventLoop{};
    InetAddress addr(PORT,IP);

    int fileFd=::open("../test/test.txt",O_RDONLY);  //fileSendFileLoop使用, 在build目錄下cmake..

    TcpServer server(loop,addr,"server",TcpServer::Option::kReusePort);

    
    //測試send
    MessageCallback messageCallback=[](const TcpConnectionPtr& tcpConnection,Buffer* buffer,Timestamp)->void{
        std::string buf(buffer->peek(),buffer->readableBytes());
        buf+="!!!!";
        buffer->retrieve(buffer->readableBytes());
        cout<<"EchoServer receive:"<<buf<<endl;
        tcpConnection->send(buf);
    };
    
    
    //測試sendFile
    /*
    MessageCallback messageCallback=[fileFd](const TcpConnectionPtr& tcpConnection,Buffer* buffer,Timestamp)->void{
        std::string buf(buffer->peek(),buffer->readableBytes());
        buf+="!!!!";
        buffer->retrieve(buffer->readableBytes());
        cout<<"EchoServer receive:"<<buf<<endl;
        
        int tmpFd=open("../test/test.txt",O_RDONLY);
        int count=lseek(tmpFd,0,SEEK_END);
        close(tmpFd);
        tcpConnection->sendFile(fileFd,0,count);
    };
    */

    ConnectionCallback connectionCallback=[](const TcpConnectionPtr &connection)->void{
        LOG_DEBUG("create TcpConnection");
    };

    ThreadInitCallback threadInitCallback=[](EventLoop* loop)->void{
        LOG_DEBUG("ThreadInitCallback");
    };

    WriteCompleteCallback writeCompleteCallback=[](const TcpConnectionPtr &connection)->void{
        LOG_DEBUG("writeCompleteCallback");
    };

    server.setThreadNum(2);
    server.setConnectionCallback(connectionCallback);
    server.setMessageCallback(messageCallback);
    server.setThreadInitCallback(threadInitCallback);
    server.setWriteCompleteCallback(writeCompleteCallback);
    server.start();

    loop->loop();
    cout<<"server exit"<<endl;
    close(fileFd);
}

void client()
{
    static int clientNum=0;
    ::sleep(1);
    cout<<"running client"<<endl;
    int fd=socket(AF_INET,SOCK_STREAM,0);
    Socket sockfd(fd);

    InetAddress addr(PORT+clientNum+1,IP);
    ++clientNum;
    sockfd.bindAddress(addr);

    InetAddress peerAddr(PORT,IP);
    cout<<"connect before"<<endl;
    if(::connect(fd,reinterpret_cast<sockaddr*>(&peerAddr),sizeof(peerAddr))!=0)
    {
        cout<<"connect error"<<endl;
    }

    //測試send函數
    char message[1024];
    snprintf(message,sizeof(message),"%s:hello world",addr.toIpPort().c_str());
    char buf[64*1024];
    while(true)
    {
        ::write(fd,message,::strlen(message));

        memset(buf,0,sizeof(buf));
        ::read(fd,buf,sizeof(buf));
        cout<<buf<<endl;
        printf("receive message:%s\n",buf);
        ::sleep(5);
    }
   
    /*
    //測試sendFile
    std::string begin="begin";
    char buf[1024];
    ::write(fd,begin.c_str(),begin.length());
    while(true)
    {
        memset(buf,0,sizeof(buf));
        ::read(fd,buf,sizeof(buf)-1);
        cout<<buf<<endl;
        ::sleep(1);
    }
    */

    ::close(fd);
    cout<<"client exit"<<endl;
}

int main()
{
    std::thread s([]()
    {
        echoServer();
    });
    std::thread c1([](){
        client();
    });
    std::thread c2([](){
        client();
    });
    s.join();
    c1.join();
    c2.join();
    printf("thread exit\n");
    return 0;
}
