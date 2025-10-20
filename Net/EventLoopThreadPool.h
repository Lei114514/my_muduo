#pragma once 

#include <vector>
#include <memory>
#include <string>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool 
{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) {numThreads_=numThreads;}

    void start(const ThreadInitCallback &cb =ThreadInitCallback());

    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoops();

    bool started() const {return started_;}
    const std::string name() const {return name_;}

private:
    EventLoop* baseLoop_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
    int numThreads_;
    int next_;

    std::string name_;
    bool started_;
};