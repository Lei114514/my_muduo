#pragma once

#include "TcpServer.h" // 假設這是你的 my_muduo 網絡庫的核心類
#include "noncopyable.h" // 假設你有一個禁止拷貝的工具類
#include <functional>
#include <string>
#include <map>

// 前置聲明，避免在頭文件中引入重量級的頭文件，減少編譯依賴
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

    TcpServer server_; 
    std::map<std::string, HttpCallback> router_;  // 路由表：存儲路徑到處理函數的映射
    HttpCallback notFoundCallback_;  // 默認的404處理回調
};