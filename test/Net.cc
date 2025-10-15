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

#define IP "192.168.232.137"
#define PORT 8899

using std::cout;
using std::endl;

void echoServer() 
{
    EventLoop* loop=new EventLoop{};
    InetAddress addr(PORT,IP);
    Acceptor acceptor{loop,addr,true};
    int fileFd=::open("../test/test.txt",O_RDONLY);  //在build目錄下cmake..

    TcpConnectionPtr clientConnection=nullptr;

    /*
    //測試send
    MessageCallback messageCallback=[](const TcpConnectionPtr& tcpConnection,Buffer* buffer,Timestamp)->void{
        std::string buf(buffer->peek(),buffer->readableBytes());
        buf+="!!!!";
        buffer->retrieve(buffer->readableBytes());
        cout<<"EchoServer receive:"<<buf<<endl;
        tcpConnection->send(buf);
    };
    */

    //測試sendFile
    MessageCallback messageCallback=[&clientConnection,fileFd](const TcpConnectionPtr& tcpConnection,Buffer* buffer,Timestamp)->void{
        std::string buf(buffer->peek(),buffer->readableBytes());
        buf+="!!!!";
        buffer->retrieve(buffer->readableBytes());
        cout<<"EchoServer receive:"<<buf<<endl;
        
        int tmpFd=open("../test/test.txt",O_RDONLY);
        int count=lseek(tmpFd,0,SEEK_END);
        close(tmpFd);
        clientConnection->sendFile(fileFd,0,count);
    };

    ConnectionCallback connectionCallback=[](const TcpConnectionPtr &connection)->void{
        LOG_DEBUG("create TcpConnection");
    };

    WriteCompleteCallback writeCompleteCallback=[](const TcpConnectionPtr &connection)->void{
        LOG_DEBUG("writeCompleteCallback");
    };

    CloseCallback closeCallback=[](const TcpConnectionPtr& connect)->void{
        LOG_DEBUG("CloseCallback");
    };

    auto newConnectionCallback=[&clientConnection,&loop,&messageCallback,&connectionCallback,&writeCompleteCallback,&closeCallback,fileFd](int fd,const InetAddress& peerAddr)->void
    {
        LOG_DEBUG("running newConnectionCallback");
        clientConnection= std::make_shared<TcpConnection>(loop,
                                                          "TcpConnection1",
                                                          fd,
                                                          InetAddress(PORT,IP),
                                                          peerAddr);

        clientConnection->setMessageCallback(messageCallback);
        clientConnection->setConnectionCallback(connectionCallback);
        clientConnection->setWriteCompleteCallback(writeCompleteCallback);
        clientConnection->setCloseCallback(closeCallback);
        clientConnection->connectEstablished();
    };

    cout<<"before setNewconnetionCallback"<<endl;
    acceptor.setNewConnectionCallback(newConnectionCallback);
    cout<<"after SetNewConnectionCallback"<<endl;
    acceptor.listen();
    cout<<"after listen"<<endl;

    loop->loop();
    cout<<"server exit"<<endl;
    close(fileFd);
}

void client()
{
    ::sleep(1);
    cout<<"running client"<<endl;
    int fd=socket(AF_INET,SOCK_STREAM,0);
    Socket sockfd(fd);

    InetAddress addr(PORT+1,IP);
    sockfd.bindAddress(addr);

    InetAddress peerAddr(PORT,IP);
    cout<<"connect before"<<endl;
    if(::connect(fd,reinterpret_cast<sockaddr*>(&peerAddr),sizeof(peerAddr))!=0)
    {
        cout<<"connect error"<<endl;
    }

    /*
    //測試send函數
    std::string message="hello world ";
    char buf[64*1024];
    while(true)
    {
        ::write(fd,message.c_str(),message.length());
        ::sleep(1);

        memset(buf,0,sizeof(buf));
        ::read(fd,buf,sizeof(buf));
        cout<<buf<<endl;
        printf("receive message:%s\n",buf);
        ::sleep(5);
    }
    */

    //測試sendFile
    std::string begin="begin";
    char buf[64*1024];
    ::write(fd,begin.c_str(),begin.length());
    while(true)
    {
        memset(buf,0,sizeof(buf));
        ::read(fd,buf,sizeof(buf)-1);
        cout<<buf<<endl;
        ::sleep(5);
    }

    ::close(fd);
    cout<<"client exit"<<endl;
}

int main()
{
    std::thread s([]()
    {
        echoServer();
    });
    std::thread c([](){
        client();
    });
    s.join();
    c.join();
    printf("thread exit\n");
    return 0;
}
