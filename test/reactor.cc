#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h> 
#include <thread>

#include "EventLoop.h"
#include "Channel.h"
#include "Timestamp.h"
#include "Logger.h"

using std::cout;
using std::endl;

// 全局的 EventLoop 指針，方便在回調中訪問
EventLoop* g_loop = nullptr;

// 定時器超時後會被調用的回調函數
void onTimerTimeout(int timerfd) {
    cout << "Timer timeout! Current time: " << Timestamp::now().toString() << endl;

    uint64_t value;
    ssize_t n = ::read(timerfd,&value,sizeof(value));
    if(n!=sizeof(value))
    {
        LOG_ERROR("read %lu bytes instead of 8",n);
    }

    // 為了讓測試能結束，我們可以在5次超時後退出事件循環
    static int count = 0;
    if (++count >= 5) {
        g_loop->quit();
    }
}

void printSomething()
{
    cout<<"hello world!!!!!!!!!!!!!!!!!!!!!!"<<endl;
    LOG_DEBUG("hello world!!!!!!!!!!!!!!!!!!!!!!");
}

int main() {
    cout << "main(): pid = " << getpid() << ", tid = " << CurrentThread::tid() << endl;
    
    // 1. 創建 EventLoop
    EventLoop loop;
    g_loop = &loop; // 將 loop 的地址賦給全局指針

    std::thread threadTestRunInLoop([]
    {
        sleep(4);
        g_loop->runInLoop(printSomething);
        printSomething();
    });

    std::thread threadQuit([]()
    {
        sleep(5);
        g_loop->quit();
    });

    // 2. 創建 timerfd
    //    CLOCK_MONOTONIC: 一個從過去某個時間點開始單調遞增的時鐘，不受系統時間修改的影響
    //    TFD_NONBLOCK | TFD_CLOEXEC: 設置為非阻塞，並在 exec 後關閉
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        std::cerr << "Failed to create timerfd" << std::endl;
        return 1;
    }

    // 3. 設置 timerfd 的超時屬性
    struct itimerspec new_value;
    //struct timespec now;
    // clock_gettime(CLOCK_MONOTONIC, &now); // 獲取當前時間
    
    // it_value: 第一次超時的時間（距離現在）
    // 我們設置為 1 秒後第一次觸發
    new_value.it_value.tv_sec = 1;
    new_value.it_value.tv_nsec = 0;

    // it_interval: 之後的周期性超時時間
    // 我們設置為每 2 秒觸發一次
    new_value.it_interval.tv_sec = 2;
    new_value.it_interval.tv_nsec = 0;
    
    if (::timerfd_settime(timerfd, 0, &new_value, nullptr) < 0) {
        std::cerr << "Failed to set timerfd" << std::endl;
        return 1;
    }

    // 4. 將 timerfd 封裝成 Channel
    Channel timerChannel(&loop, timerfd);
    
    // 5. 設置讀回調函數
    timerChannel.setReadCallback([timerfd](Timestamp)
    {
        onTimerTimeout(timerfd);
    });
    
    // 6. 啟用 Channel 的讀事件監聽
    timerChannel.enableReading();

    cout << "Starting event loop..." << endl;

    // 7. 啟動事件循環！
    loop.loop();

    // 8. loop() 退出後，關閉文件描述符
    ::close(timerfd);

    cout << "Event loop finished." << endl;

    return 0;
}