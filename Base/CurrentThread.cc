#include "CurrentThread.h"

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    thread_local int t_cacheTid=0;

    int cacheTid()
    {
        t_cacheTid=static_cast<int>(::syscall(SYS_gettid));
        return t_cacheTid;
    }
}

// #include <iostream>
// int main()
// {
//     std::cout<<CurrentThread::tid()<<std::endl;
//     return 0;
// }