## 项目介绍
本项目是参考muduo实现的基于主从Reactor模型的多线程网络库。使用C++11编写去除muduo对boost的依赖。

---
## 开发环境
- Ubuntu 22.04.2 LTS
- gcc (Ubuntu 11.4.0-1ubuntu1~22.04.2) 11.4.0
- cmake version 3.22.1

---
## 功能板块
基础板块:
- Logger: 日志板块, 有多个日志等级, DEBUG级可以自定义是否打印
- Noncopyable: 删除拷贝函数和赋值运算符的类, 对于不想被复制的类可以继承它
- CurrentThread.h: 通过线程局部存储(thread_local)缓存当前线程的id, 那么只有第一次需要使用系统调用来得到线程的id
- Timestamp: 时间类

Reactor模型:
- Channel: 封装事件的处理
	- 触发事件时使用对应的回调函数
	- 提供判断回调函数的对象是否有效(weak_ptr)
	- 提供对事件的修改和查询
- Poller: 封装多路IO复用
	- 等待事件的发生
	- 提供修改期待事件的类型 
- EPoller: 对于Linux的Poller的实现
	- 实现Poller的函数
	- 返回发生事件的Channel列表
- EventLoop: 封装整个事件循环
	- 不断循环多路IO复用和处理异步任务
	- 提供channel和poller的通信

网络组件:
- InetAddress: 封装sockaddr_in 
	- 提供读取sockaddr_in的信息
- Socket: 使用RAII手法封装了socket文件描述符
	- 封装底层的 socket API 操作, 包括bind、listen、acceptor、设置网络属性、关闭连接等功能
- Buffer: 封装缓冲区, 通过一个vector和两个索引, 构建了"prependable + readable + writable" 的内存布局, 来模拟缓冲区
	- 实现写入、读出和扩容
- Acceptor: 封装acceptor系统调用, 专门用于接受新的TCP连接
- TcpConnection: 封装TCP连接
	- 管理连接, 建立连接、收发数据、关闭连接、销毁连接等功能
	- 在关键的位置使用回调函数
- TcpServer: 负责管理 Acceptor 和所有的 TcpConnection。它将新接受的连接通过轮询算法分发给线程池中的I/O线程处理。

多线程板块:
- Thread: 封装线程
- EventLoopThread: 封装线程的操作  
- EventLoopThreadPool: 封装线程池

----
## 项目运行


---
## 致谢
https://github.com/youngyangyang04/muduo-core
