#include <unistd.h>
#include <sys/sendfile.h>
#include <memory>

#include "TcpConnection.h"
#include "Channel.h"
#include "Socket.h"
#include "Logger.h"
#include "Buffer.h"
#include "EventLoop.h"

EventLoop* checkLoopNotNull(EventLoop* loop)
{
    if(loop==nullptr)
    {
        LOG_FATAL("mainLoop is NULL");
    
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop
            , const std::string &nameArg
            , int sockfd
            , const InetAddress &localAddr
            , const InetAddress &peerAddr)
    : loop_(checkLoopNotNull(loop))
    , channel_(new Channel{loop,sockfd})
    , socket_(new Socket{sockfd})
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_( 64 * 1024 * 1024)
    , name_(nameArg)
    , state_(StateE::kConnected)
    , reading_(true)
    , inputBuffer_()
    , outputBuffer_()
{
    channel_->setReadCallback([this](Timestamp timestamp)->void{
        handleRead(timestamp);
    });
    channel_->setWriteCallback([this]()->void{
        handleWrite();
    });
    channel_->setCloseCallback([this]()->void{
        handleClose();
    });
    channel_->setErrorCallback([this]()->void{
        handleError();
    });
    LOG_DEBUG("TcpConnction create, fd=%d, name=%s",sockfd,name_.c_str());
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG("TcpConnect Destory, fd=%d, name=%s, state=%d",socket_->fd(),name_.c_str(),static_cast<int>(state_.load()));
}

void TcpConnection::send(Buffer* buf)
{
    if(connected())
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(),buf->readableBytes());
            buf->retrieveAll();
        }
        else 
        {
            std::shared_ptr<Buffer> new_buf = std::make_shared<Buffer>();
            new_buf->swap(*buf);

            loop_->runInLoop([shared_this = shared_from_this(),
                              moved_buf = std::move(new_buf)] () mutable -> void
            {
                shared_this->sendInLoop(moved_buf->peek(),moved_buf->readableBytes());
            });
        }
    }
}
void TcpConnection::send(const std::string &buf)
{
    if(connected())
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(),buf.length());
        }
        else 
        {
            loop_->runInLoop([shared_this=shared_from_this(),buf]()->void{
                shared_this->sendInLoop(buf.c_str(),buf.length());
            });
        }
    }
}
void TcpConnection::sendInLoop(const void* data,size_t len)
{
    LOG_DEBUG("begin sendInLoop,channel isWriting=%d, outputBuffer readableBytes=%ld",channel_->isReading(),outputBuffer_.readableBytes());
    size_t nwrote = 0;
    size_t remaining = len;
    int fault = 0;

    if(state_ == StateE::kDisconnectied)
    {
        LOG_ERROR("connection disconnected");
        return ;
    }

    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0 )
    {
        LOG_DEBUG("faster send message,data=%s",data);
        nwrote=::write(channel_->fd(),data,len);
        if(nwrote>0)
        {
            remaining-=nwrote;
            if(remaining==0&&writeCompleteCallback_!=nullptr)
            {
                writeCompleteCallback_(shared_from_this());
                LOG_DEBUG("complete sendInLoop");
            }
        }
        else 
        {
            nwrote=0;
            if(errno!=EAGAIN)
            {
                int saveErrno=errno;
                LOG_ERROR("write error, errno=%d",saveErrno);
            }
            if(errno==EPIPE || errno== ECONNRESET)
            {
                fault=true;
            }
        }
    } 

    if(remaining>0&&!fault)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        if(oldLen+remaining>=highWaterMark_&&oldLen<highWaterMark_&&highWaterMarkCallback_)
        {
            LOG_DEBUG("high water");
            highWaterMarkCallback_(shared_from_this());
        }

        LOG_DEBUG("data append outputBuffer");
        outputBuffer_.append(static_cast<const char*>(data)+nwrote,remaining);
        if(!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }

    if(fault)
    {
        handleError();
    }
}
void TcpConnection::sendFile(int fd,off_t offset,size_t count)
{
    if(connected())
    {
        if(loop_->isInLoopThread())
        {
            sendFileInLoop(fd,offset,count);
        }
        else 
        {
            loop_->runInLoop([shared_this=shared_from_this(),fd,offset,count]()->void{
                shared_this->sendFileInLoop(fd,offset,count);
            });
        }
    }
    else 
    {
        LOG_ERROR("TcpConnection not connected");
    }
}
void TcpConnection::sendFileInLoop(int fd,off_t offset,size_t count)
{
    LOG_DEBUG("begin sendFileInLoop");
    size_t nwrote=0;
    size_t remaining=count;
    int fault=0;

    if(state_ == StateE::kDisconnecting)
    {
        LOG_ERROR("connection disconnected");
        return;
    }

    if(!channel_->isWriting()&&outputBuffer_.readableBytes()==0)
    {
        nwrote = ::sendfile(socket_->fd(),fd,&offset,count);
        if(nwrote>=0)
        {
            remaining-=nwrote;
            if(remaining==0&&writeCompleteCallback_!=nullptr)
            {
                writeCompleteCallback_(shared_from_this());
                LOG_DEBUG("complete sendFileInLoop");
            }
        }
        else 
        {
            nwrote=0;
            if(errno!=EAGAIN)
            {
                int saveErrno=errno;
                LOG_ERROR("write error, errno=%d",saveErrno);
            }
            if(errno==EPIPE||errno==ECONNRESET)
            {
                fault=true;
            }
        }
    }

    if(!fault&&remaining!=0)
    {
        LOG_DEBUG("file sending not complete, remaining=%ld",remaining);
        fileInfo_=std::make_unique<FileInfo>(fd,static_cast<off_t>(offset+nwrote),remaining);
        if(!channel_->isReading())
        {
            channel_->enableReading();
        }
    }

    if(fault)
    {
        handleError();
    }
}
    
void TcpConnection::shutdown()
{
    if(state_==StateE::kConnected)
    {
        setState(StateE::kDisconnecting);
        if(loop_->isInLoopThread())
        {
            shutdownInLoop();
        }
        else
        {
            loop_->queueInLoop([shared_this=shared_from_this()]()->void{
                shared_this->shutdownInLoop();
            });
        }
    }
}

void TcpConnection::connectEstablished()
{
    setState(StateE::kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}
void TcpConnection::connectDestroyed()
{
    if(state_==StateE::kConnected)
    {
        setState(StateE::kDisconnectied);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}


void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::handleWrite()
{
    if(!channel_->isWriting())
    {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing",channel_->fd());
        return;
    }
    
    if(fileInfo_==nullptr)
    {
        LOG_DEBUG("sendInLoop");
        int saveErrno=0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(),&saveErrno);
        if(n<0)
        {
            LOG_ERROR("send EAGAIN");
            if(errno!=EAGAIN)
            {
                int savaErrno = errno;
                LOG_ERROR("send errno=%d",savaErrno);
                handleError();
            }
        }
        else 
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes()==0)
            {
                channel_->disableWriting();
                if(writeCompleteCallback_!=nullptr)
                {
                    writeCompleteCallback_(shared_from_this());
                }
                if(state_==StateE::kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
    }
    else 
    {
        LOG_DEBUG("sendFileInLoop");
        off_t offset=fileInfo_->offset;   
        ssize_t n=::sendfile(channel_->fd(),fileInfo_->fd,&(fileInfo_->offset),fileInfo_->count);
        if(n<0)
        {
            LOG_ERROR("sendfile EAGAIN");
            if(errno!=EAGAIN)
            {
                int savaErrno = errno;
                LOG_ERROR("sendfile errno=%d",savaErrno);
                fileInfo_.reset();
                handleError();
            }
        }
        else 
        {
            fileInfo_->offset+=n;
            fileInfo_->count-=n;
            if(fileInfo_->count==0)
            {
                fileInfo_.reset();

                if(outputBuffer_.readableBytes()==0)
                {
                    channel_->disableWriting();
                }

                if(writeCompleteCallback_!=nullptr)
                {
                    writeCompleteCallback_(shared_from_this());
                }
            }
        }
    }
}
void TcpConnection::handleRead(Timestamp receiveTime)
{
    LOG_DEBUG("TcpConnection handleRead");
    int saveErrno=0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(),&saveErrno);
    if( n > 0 )
    {
        LOG_DEBUG("TcpConnection receive message");
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }
    else if ( n == 0 )
    {
        LOG_DEBUG("TcpConnection close, name=%s",name_.c_str());
        handleClose();
    }
    else 
    {
        LOG_ERROR("Tcpconnection error, fd=%d",saveErrno);
        errno=saveErrno;
        handleError();
    }
}
void TcpConnection::handleClose()
{
    LOG_DEBUG("TcpConnection handleClose fd=%d, name=%s, state=%d",socket_->fd(),name_.c_str(),static_cast<int>(state_.load()));
    setState(StateE::kDisconnectied);
    channel_->disableAll();

    TcpConnectionPtr share_this(shared_from_this());
    connectionCallback_(share_this);
    closeCallback_(share_this);

}
void TcpConnection::handleError()
{
    int optval=0;
    socklen_t optlen=sizeof(optval);
    int saveErrno=0;
    if(::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&optval,&optlen)<0)
    {
        saveErrno=errno;
    }
    else
    {
        saveErrno=optval;
    }
    LOG_ERROR("TcpConnectino error, name=%s, errno=%d",name_.c_str(),errno);
}



