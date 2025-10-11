#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>

#include "Acceptor.h"
#include "Logger.h" 
#include "InetAddress.h"

int createSocket()
{
    int fd=::socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,IPPROTO_TCP);
    if(fd<0)
    {
        LOG_FATAL("acceptor socket error, errno=%d",errno);
    }
    return fd;
}

Acceptor::Acceptor(EventLoop* loop,const InetAddress& listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createSocket())
    , acceptChannel_(loop_,acceptSocket_.fd())
    , listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);

    acceptChannel_.setReadCallback([this](Timestamp)
    {
        handleRead();
    });
}
Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    LOG_DEBUG("acceoptor listenning");
    listenning_=true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    LOG_DEBUG("running Acceptor handleRead");
    InetAddress peerAddr{};
    while(true)
    {
        int connfd=acceptSocket_.accept(&peerAddr);
        if(connfd>=0)
        {
            if(NewConnectionCallback_!=nullptr)
            {
                NewConnectionCallback_(connfd,peerAddr);
            }
            else 
            {
                ::close(connfd);
            }
        }
        else
        {

            if(errno==EAGAIN||errno==EWOULDBLOCK)
            {
                LOG_DEBUG("all pending connection accepted");
                break;
            }
            else 
            {
                LOG_ERROR("accept error:%d\n",errno);
                if(errno==EMFILE)
                {
                    LOG_ERROR("scokfd reached limit");
                }
                break;
            }

        }
    }
}

//測試代碼在../test/Net.cc

