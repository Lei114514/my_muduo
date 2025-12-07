#pragma once

#include "../Net/TcpServer.h" 
#include "noncopyable.h" 
#include <functional>
#include <string>
#include <map>

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable
{
public:
    // 定義業務邏輯回調函數的類型
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string& name = "MyHttpServer");
    ~HttpServer()=default; 

    /// @brief 啟動服務器（開始監聽）
    void start();

    /// @brief 設置處理HTTP請求的回調函數
    /// @param path 請求的路徑，例如 "/" 或 "/hello"
    /// @param cb 對應該路徑的處理函數
    void setHttpCallback(const std::string& path, const HttpCallback& cb);

    /// @brief 設置處理所有未找到路徑（404）的默認回調
    void setNotFoundCallback(const HttpCallback& cb);

    void setThreadNum(int threadNum) { server_.setThreadNum(threadNum); }

private:
    /// @brief TcpServer 的 onConnection 回調
    void onConnection(const TcpConnectionPtr& conn);

    /// @brief TcpServer 的 onMessage 回調，這是HTTP協議解析的核心驅動
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
    
    // 內部輔助函數，當請求解析完成後被調用
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest& req);

    // TODO: 實現定時器來斷開不常用的連接
    void onTime();

    TcpServer server_; 
    std::map<std::string, HttpCallback> router_;  // 路由表：存儲路徑到處理函數的映射
    HttpCallback notFoundCallback_;  // 默認的404處理回調
};