#pragma once

#include <netinet/in.h>
#include <netinet/ip.h> 
#include <string>

class InetAddress
{
public:
    explicit InetAddress(uint16_t port=0,std::string ip="127.0.0.1");
    explicit InetAddress(const sockaddr_in& address) 
        : address_(address)
    {
    }

    uint16_t toPort() const;
    std::string toIp() const;
    std::string toIpPort() const;

    const sockaddr_in* getSockAddr() const { return &address_; }
    void setSockAddr(const sockaddr_in& address) { address_ = address; }
    
private:
    struct sockaddr_in address_;
};