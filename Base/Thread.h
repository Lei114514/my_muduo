#pragma once

#include <thread>
#include <memory>
#include <functional>
#include <string>
#include <atomic>

#include "noncopyable.h"

class Thread : private noncopyable
{
public:
    using ThreadFunc=std::function<void()>;

    explicit Thread(ThreadFunc, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() {return started_;}
    bool tid() {return tid_;}
    const std::string& name() const {return name_;}

    static int numCreated() {return numCreated_;}

private:
    void setDefaultName();

    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;

    bool started_;
    bool joined_;
    std::string name_;
    static std::atomic_int numCreated_;
};