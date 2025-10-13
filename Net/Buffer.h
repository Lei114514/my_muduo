#pragma once

#include <vector>
#include <stddef.h>
#include <string>
#include <algorithm>

class Buffer
{
public:
    static const size_t kCheapPrepend=8;
    static const size_t kInitialSize=1024;

    Buffer(size_t initalSize=kInitialSize)
    : buffer_(initalSize+kCheapPrepend)
    , readerIndex_(kCheapPrepend)
    , writerIndex_(kCheapPrepend)
    {
    }

    size_t readableBytes() const {return writerIndex_ - readerIndex_; }
    size_t writableBytes() const {return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const {return readerIndex_; }

    char* beginWrite() {return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }
    const char* peek() const {return begin() + readerIndex_; }

    void retrieve(size_t len)
    {
        if(len<readableBytes())
        {
            readerIndex_+=len;
        }
        else 
        {
            retrieveAll();
        }
    }
    void retrieveAll()
    {
        readerIndex_=kCheapPrepend;
        writerIndex_=kCheapPrepend;
    }

    void ensureWritableBytyes(size_t len)
    {
        if( len > writableBytes() )
        {
            makeSpace(len);
        }
    }
    void append(const char* data,size_t len)
    {
        ensureWritableBytyes(len);
        std::copy(data,data+len,beginWrite());
        writerIndex_+=len;
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(),len);
        retrieve(len);
        return result;
    }
    std::string retrieveAllString() {return retrieveAsString(readableBytes()); }

    ssize_t readFd(int fd, int* saveErrno);
    ssize_t writeFd(int fd, int* saveErrno);

private:
    char* begin() {return buffer_.data(); }
    const char* begin() const {return buffer_.data(); }

    void makeSpace(size_t len)
    {
        if( prependableBytes() + writableBytes() >= len + kCheapPrepend )
        {
            size_t readable=readableBytes();
            std::copy(begin()+readerIndex_, 
                      begin()+writerIndex_, 
                      begin()+kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
        else 
        {
            buffer_.resize(buffer_.size() + len);
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};