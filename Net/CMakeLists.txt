set(NET_SOURCES 
    Channel.cc
    DefaultPoller.cc
    EPollPoller.cc
    EventLoop.cc
    Poller.cc
    
    InetAddress.cc
    Socket.cc
    Acceptor.cc
    Buffer.cc
    TcpConnection.cc
    TcpServer.cc

    EventLoopThread.cc
    EventLoopThreadPool.cc
    #每實現一個就新增一個
)

add_library(Net STATIC ${NET_SOURCES})

target_include_directories(Net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Net PUBLIC Base)