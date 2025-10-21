#include "TcpServer.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include "Logger.h"
#include "Callbacks.h"
#include <string>

#define PORT 8899

#include <iostream>

class EchoServer
{
public:
    EchoServer(EventLoop* loop, const InetAddress& addr, const std::string &name)
        : server_(loop,addr,name,TcpServer::Option::kReusePort)
        , loop_(loop)
    {
        server_.setThreadNum(3);
        server_.setConnectionCallback([this](const TcpConnectionPtr& conn)->void{
            connectionCallback(conn);
        });
        server_.setMessageCallback([this](const TcpConnectionPtr& tcpConnection,Buffer* buffer,Timestamp time)->void{
            messageCallback(tcpConnection,buffer,time);
        });
        server_.setThreadInitCallback([this](EventLoop* loop)->void{
            threadInitCallback(loop);
        });
        server_.setWriteCompleteCallback([this](const TcpConnectionPtr& conn)->void{
            writeCompleteCallback(conn);
        });
    }
    ~EchoServer()
    {

    }

    void setThreadNum(int threadNum)
    {
        server_.setThreadNum(threadNum);
    }

    void start()
    {
        server_.start();
        loop_->loop();
    }

private:   
    void messageCallback(const TcpConnectionPtr& tcpConnection,Buffer* buffer,Timestamp)
    {
        LOG_DEBUG("readableBytes=%d",buffer->readableBytes());
        std::string data =buffer->retrieveAllString();
        tcpConnection->send(data);
    }

    void connectionCallback(const TcpConnectionPtr &connection)
    {
        if(connection->connected())
        {
            LOG_DEBUG("connection up:%s",connection->peerAddress().toIpPort().c_str());
        }
        else 
        {
            LOG_DEBUG("connection down:%s",connection->peerAddress().toIpPort().c_str());
        }
    };

    void threadInitCallback(EventLoop* loop)
    {
        LOG_DEBUG("ThreadInitCallback");
    };

    void writeCompleteCallback(const TcpConnectionPtr &connection)
    {
        LOG_DEBUG("writeCompleteCallback");
    };

    TcpServer server_;
    EventLoop* loop_;
};

int main()
{
    EventLoop* loop=new EventLoop();
    InetAddress localAddr{PORT};
    EchoServer echoServer(loop,localAddr,"EchoServer");
    echoServer.setThreadNum(1);
    echoServer.start();
    return 0;
}