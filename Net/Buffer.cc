#include <unistd.h>
#include <sys/uio.h>

#include "Buffer.h"
#include "Logger.h"

ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extraBuf[65536]={0};

    struct iovec vec[2];
    const size_t writable = writableBytes();

    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;

    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf); 

    const int iovcnt=(writable>sizeof(extraBuf) ? 1 : 2 );
    const ssize_t n = ::readv(fd,vec,iovcnt);
    if( n < 0 )
    {
        *saveErrno = errno;
        LOG_ERROR("buffer readv error, fd=%d, errno=%d",fd,*saveErrno);
    }
    else if( n < writable )
    {
        writerIndex_ += n;
    }
    else
    {
        writerIndex_ = buffer_.size();
        append(extraBuf,n-writable);
    }

    return n;
}
ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd,peek(),readableBytes());
    if(n<0)
    {
        *saveErrno=errno;
    }
    return n;
}