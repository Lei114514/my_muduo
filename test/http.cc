#include <iostream>
#include <string.h>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServer.h"
#include "Buffer.h"
#include "Logger.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "InetAddress.h"

const uint16_t port=8899; 

void echoCallback(const HttpRequest& request, HttpResponse* response)
{
    // 獲取請求信息
    const std::string& method = std::to_string(static_cast<int>(request.getMethod()));
    const std::string& path = request.getPath();
    const std::string& version = request.getVersion();
    const std::string& body = request.getBody();
    
    const auto& headers = request.getHeaders();
    
    // 構建 echo 響應
    std::string echoResponse;

    // 響應頭部
    echoResponse += "HTTP Echo Server Response\n";
    echoResponse += "=======================\n";
    echoResponse += "Method: " + method + "\n";
    echoResponse += "Path: " + path + "\n";
    echoResponse += "Version: " + version + "\n";
    echoResponse += "\n";

    // 請求頭
    echoResponse += "Request Headers:\n";
    for (const auto& header : headers) {
        echoResponse += "  " + header.first + ": " + header.second + "\n";
    }
    echoResponse += "\n";

    // 請求體
    if (!body.empty()) {
        echoResponse += "Request Body:\n";
        echoResponse += "----------------------------------------\n";
        echoResponse += body;
        echoResponse += "\n----------------------------------------\n";
    } else {
        echoResponse += "Request Body: (empty)\n";
    }

    // 設置響應
    response->setStatusCode(HttpResponse::k200Ok);
    response->setContentType("text/plain; charset=utf-8");
    response->setBody(echoResponse);

    // 添加響應頭
    response->addHeader("Server", "EchoServer/1.0");
    response->addHeader("X-Powered-By", "muduo");
    response->addHeader("Content-Length", std::to_string(echoResponse.length()));

    // 根據 Connection 頭決定是否保持連接
    const std::string& connectionHeader = request.getHeader("Connection");
    if (connectionHeader == "keep-alive" ||
        (version == "HTTP/1.1" && connectionHeader != "close")) {
        response->addHeader("Connection", "keep-alive");
    } else {
        response->addHeader("Connection", "close");
    }
}

int main()
{
    EventLoop* loop = new EventLoop();
    InetAddress addr{port, "0.0.0.0"};
    HttpServer server{loop, addr, "EchoServer"};

    // 註冊 echo 回調到所有路徑
    // 使用萬用字元路由處理所有路徑
    server.setHttpCallback("/", [](const HttpRequest& request, HttpResponse* response)->void {
        echoCallback(request, response);
    });
    
    // 也可以註冊特定路徑的 echo
    server.setHttpCallback("/echo", [](const HttpRequest& request, HttpResponse* response)->void {
        echoCallback(request, response);
    });
    
    server.setHttpCallback("/api/echo", [](const HttpRequest& request, HttpResponse* response)->void {
        echoCallback(request, response);
    });

    server.setThreadNum(4);
    server.start();

    loop->loop();

    delete loop;
    return 0;
}