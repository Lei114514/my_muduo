#include <iostream>
#include <sys/types.h>    
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc,char* argv[])
{
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd<0)
    {
        perror("socker error");
        return -1;
    }

    struct sockaddr_in peerAddr;
    peerAddr.sin_family=AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&peerAddr.sin_addr);
    peerAddr.sin_port=htons(8899);
    if(::connect(fd,reinterpret_cast<sockaddr*>(&peerAddr),sizeof(peerAddr))<0)
    {
        perror("connect error");
    }

    char message[1024];
    char rbuf[1024];
    while(true)
    {
        ::fgets(message,sizeof(message)-1,stdin);
        message[strlen(message)-1]='\0'; 


        ::write(fd,message,strlen(message));

        memset(rbuf,0,sizeof(rbuf));
        ::read(fd,rbuf,sizeof(rbuf));
        printf("echoServer:%s\n",rbuf);

        if(strcmp(message,"quit")==0)
        {
            break;
        }
    }
    ::close(fd);
    return 0;
}