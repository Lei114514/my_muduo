#include <memory>
#include <string>
#include <string.h>

#include "TcpServer.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "Logger.h"
#include "EventLoopThreadPool.h"

static EventLoop* checkLoopNotNull(EventLoop* loop)
{
    if(loop==nullptr)
    {
        LOG_FATAL("loop is NULL");
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop
            ,const InetAddress &listenAddr
            ,const std::string &name
            ,Option option)
    : loop_(checkLoopNotNull(loop))
    , acceptor_(std::make_unique<Acceptor>(loop,listenAddr,(option==Option::kReusePort?1:0)))
    , ipPort_(listenAddr.toIpPort())
    , nextConnId_(1)
    , name_(name)
    , started_(false)
    , threadPool_(std::make_shared<EventLoopThreadPool>(loop,name))
{
    acceptor_->setNewConnectionCallback([this](int sockfd,const InetAddress& peerAddr)->void{
        newConnection(sockfd,peerAddr);
    });
}
TcpServer::~TcpServer()
{
    for(auto &item:connections_)
    {
        TcpConnectionPtr conn{item.second};
        item.second.reset();
        conn->getLoop()->runInLoop([conn]()->void{
            conn->connectDestroyed();
        });
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    numThreads_=numThreads;
    threadPool_->setThreadNum(numThreads_);
}

void TcpServer::start()
{
    if(started_.fetch_add(1)==0)
    {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop([this]()->void{
            acceptor_->listen();
        });
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    EventLoop* loop = threadPool_->getNextLoop();

    char buf[64];
    snprintf(buf,sizeof(buf),"-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName=name_+buf;

    LOG_DEBUG("TcpServer:%s - new Connection [%s] from %s",name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

    sockaddr_in addr;
    ::memset(&addr,0,sizeof(addr));
    socklen_t optlen=sizeof(addr);
    if(::getsockname(sockfd,reinterpret_cast<sockaddr*>(&addr),&optlen)<0)
    {
        LOG_ERROR("getsockname error, name=%s",connName.c_str());
    }
    InetAddress localAddr(addr);

    TcpConnectionPtr conn=std::make_shared<TcpConnection>(loop,
                                                          connName,
                                                          sockfd,
                                                          localAddr,
                                                          peerAddr);
    connections_[connName]=conn;

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    
    conn->setCloseCallback([this](const TcpConnectionPtr& connection)->void{
        removeConnection(connection);
    });

    loop->runInLoop([conn]()->void{
        conn->connectEstablished();
    });
}
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop([this,conn]()->void{
        removeConntionInLoop(conn);
    });
}
void TcpServer::removeConntionInLoop(const TcpConnectionPtr& conn)
{
    LOG_DEBUG("TcpServer:%s - remove Connection [%s]",name_.c_str(),conn->name().c_str());
    
    connections_.erase(conn->name());
    EventLoop* loop=conn->getLoop();
    loop->queueInLoop([conn]()->void{
        conn->connectDestroyed();
    });
}
