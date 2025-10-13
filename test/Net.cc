#include <iostream>
#include <thread>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "Acceptor.h"
#include "Logger.h" 
#include "InetAddress.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "Buffer.h"

#define IP "192.168.232.137"
#define PORT 8899

EventLoop* g_loop=new EventLoop{};

using std::cout;
using std::endl;

void server()
{
    InetAddress addr(PORT,IP);
    Acceptor acceptor{g_loop,addr,true};

    std::shared_ptr<Channel> clientChannel = nullptr;  //使用share_ptr
    Buffer buffer{};
    int readfd=-1;

    auto readEventCallback=[&clientChannel,&buffer](Timestamp)->void{
        int saveErrno = 0;
        buffer.readFd(clientChannel->fd(),&saveErrno);
        cout<<buffer.retrieveAsString(buffer.readableBytes())<<endl;
    };
    auto writeEventCallback=[]()
    {
        //測試Acceptor不需要寫
    };

    auto newConnectionCallback=[&clientChannel,readEventCallback,writeEventCallback,&readfd](int fd,const InetAddress& peerAddr)->void
    {
        LOG_DEBUG("running newConnectionCallback");
        clientChannel= std::make_shared<Channel>(g_loop,fd);
        clientChannel->setReadCallback(readEventCallback);
        clientChannel->enableReading();
        //clientChannel->setWriteCallback(writeEventCallback);
        //clientChannel->enableWriting();
    };
    cout<<"before setNewconnetionCallback"<<endl;
    acceptor.setNewConnectionCallback(newConnectionCallback);
    cout<<"after SetNewConnectionCallback"<<endl;
    acceptor.listen();
    cout<<"after listen"<<endl;

    g_loop->loop();
    cout<<"server exit"<<endl;
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

    
    std::string message="hello world ";
    while(true)
    {
        message+=message;
        ::write(fd,message.c_str(),message.length());
        ::sleep(1);
        printf("send message, size=%d\n",message.length());
    }

    ::close(fd);
    cout<<"client exit"<<endl;
}

int main()
{
    std::thread s([]()
    {
        server();
    });
    std::thread c([](){
        client();
    });
    s.join();
    c.join();
    printf("thread exit\n");
    return 0;
}
