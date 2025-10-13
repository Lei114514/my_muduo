#include <functional>
#include <memory>

class TcpConnection;
class Buffer;
class Timestamp;

using TcpConnectionPtr=std::shared_ptr<TcpConnection>;
using ConnectionCallback=std::function<void(const TcpConnection&)>;
using CloseCallback=std::function<void(const TcpConnection&)>;
using WriteCompleteCallback=std::function<void(const TcpConnection&)>;
using HighWaterMarkCallback=std::function<void(const TcpConnection&)>;

using MessageCallback=std::function<void(const TcpConnection&,
                                        Buffer*,
                                        Timestamp)>;