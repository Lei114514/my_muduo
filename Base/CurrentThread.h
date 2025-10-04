#pragma once

namespace CurrentThread
{
    extern thread_local int t_cacheTid;  //這裡不使用pid_t是因為pid_t需要引入頭文件, 而CurrentThread.h是一個很底層的頭文件, 在Linux下pid_t實際上是一個int, 那麼雖然降低了可讀性, 但是可以避免編譯依賴

    int cacheTid();

    inline int tid()
    {
        if(t_cacheTid==0)[[unlikely]]
        {
            cacheTid();
        }
        return t_cacheTid;
    }
}