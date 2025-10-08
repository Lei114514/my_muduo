#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>

#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"

const int kMaxListenQueue=1024;

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localAddr)
{
    if(bind(sockfd_,reinterpret_cast<const struct sockaddr*>(localAddr.getSockAddr()),sizeof(sockaddr_in)))
    {
        LOG_FATAL("bind sockfd:%d fail, address=%s",sockfd_,localAddr.toIpPort().c_str());
    }
}
void Socket::listen()
{
    if(::listen(sockfd_,kMaxListenQueue)!=0)
    {
        LOG_FATAL("listen sockfd=%d fail",sockfd_);
    }
}
int Socket::accept(InetAddress* peerAddr)
{
    sockaddr_in addr;
    ::memset(&addr,0,sizeof(addr));
    socklen_t len=sizeof(addr);
    
    int connfd=::accept4(sockfd_,reinterpret_cast<struct sockaddr*>(&addr),&len,SOCK_NONBLOCK|SOCK_CLOEXEC);
    if(connfd>0)
    {
        peerAddr->setSockAddr(addr);
    }
    else 
    {
        if(errno!=EAGAIN&&errno!=EWOULDBLOCK)
        {
            LOG_ERROR("accept error sockfd=%d",sockfd_);
        }
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if(::shutdown(sockfd_,SHUT_WR)!=0)
    {
        LOG_ERROR("shutdown error, sockfd_=%d",sockfd_);
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof(optval));
}
void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}
void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}
void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

// g++ -I ./ -I../Base Socket.cc InetAddress.cc ../Base/Logger.cc ../Base/Timestamp.cc
// #include <iostream>
// #include <thread>

// #define IP "192.168.232.137"
// #define PORT 8899

// void server()
// {
//     int fd=socket(AF_INET,SOCK_STREAM,0);
//     Socket sockfd(fd);
//     InetAddress addr(PORT);
//     sockfd.bindAddress(&addr);
//     sockfd.listen();

//     InetAddress peerAddr;
//     int connfd=sockfd.accept(&peerAddr);

//     char buf[128]={0};
//     read(connfd,buf,sizeof(buf));
//     printf("%s\n",buf);
// }

// void client()
// {
//     int fd=socket(AF_INET,SOCK_STREAM,0);
//     Socket sockfd(fd);

//     InetAddress addr(PORT+1);
//     sockfd.bindAddress(&addr);

//     InetAddress peerAddr(PORT);
//     connect(fd,reinterpret_cast<sockaddr*>(&peerAddr),sizeof(peerAddr));

//     const char buf[]="hello world";
//     write(fd,buf,sizeof(buf));
// }

// int main()
// {
//     std::thread s([]()
//     {
//         server();
//     });
//     std::thread c([](){
//         client();
//     });
//     s.join();
//     c.join();
//     printf("thread exit\n");
//     return 0;
// }