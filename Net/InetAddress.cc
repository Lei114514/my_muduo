#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include "InetAddress.h"

InetAddress::InetAddress(uint16_t port,std::string ip)
{
    ::memset(&address_,0,sizeof(address_));
    address_.sin_family = AF_INET;
    address_.sin_port = htons(port);
    inet_pton(AF_INET,ip.c_str(),&address_.sin_addr);
}


uint16_t InetAddress::toPort() const
{
    return ::ntohs(address_.sin_port);
}
std::string InetAddress::toIp() const
{
    char address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&address_.sin_addr,address,sizeof(address));
    return address;
}
std::string InetAddress::toIpPort() const
{
    char IpPort[64];
    inet_ntop(AF_INET,&address_.sin_addr,IpPort,sizeof(IpPort));
    size_t end = ::strlen(IpPort);
    uint16_t port=::ntohs(address_.sin_port);
    sprintf(IpPort+end,":%u",port);
    return IpPort;
}

// #include <iostream>
// using std::cout;
// using std::endl;
// int main()
// {
//     uint16_t port=9988;
//     InetAddress address(port);
//     cout<<address.toPort()<<endl;
//     cout<<address.toIp()<<endl;
//     cout<<address.toIpPort()<<endl;

//     // sockaddr_in addrin;
//     // ::memset(&addrin,0,sizeof addrin);
//     // addrin.sin_family=AF_INET;
//     // addrin.sin_port=htons(9988);
//     // inet_pton(AF_INET,"192.168.232.134",&addrin.sin_addr);
//     //InetAddress address(addrin);
//     // cout<<address.toPort()<<endl;
//     // cout<<address.toIp()<<endl;
//     // cout<<address.toIpPort()<<endl;

//     return 0;
// }