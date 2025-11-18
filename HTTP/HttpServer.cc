#include "HttpServer.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "Timestamp.h"
#include "TcpConnection.h"
#include "Logger.h"
#include "InetAddress.h"

HttpServer::HttpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& name = "MyHttpServer")
    : server_(loop,listenAddr,name,TcpServer::Option::kReusePort)
{
    server_.setConnectionCallback([this](const TcpConnectionPtr& conn)->void{
        onConnection(conn);
    });
    server_.setMessageCallback([this](const TcpConnectionPtr& conn,Buffer* buf,Timestamp time)->void{
        onMessage(conn,buf,time);
    });
}

void HttpServer::start()
{
    server_.start();
}

void HttpServer::setHttpCallback(const std::string& path, const HttpCallback& cb)
{
    router_[path]=cb;
}

void HttpServer::setNotFoundCallback(const HttpCallback& cb)
{
    notFoundCallback_=cb;
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if(conn->connected())
    {
        LOG_DEBUG("New connection from %s",conn->peerAddress().toIpPort());
        conn->setContext(HttpRequest());
    }
    else 
    {
        LOG_DEBUG("Connection from %s is down",conn->peerAddress().toIpPort());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    std::any* context = conn->getMutableContext();
    if(context==nullptr)
    {
        LOG_ERROR("context in null on connection %s",conn->name().c_str());
        conn->shutdown();
        return;
    }

    HttpRequest* request = std::any_cast<HttpRequest>(context);
    if(request==nullptr)
    {
        LOG_ERROR("context is not HttpRequest type");
        conn->shutdown();
        return; 
    }

    if(request->parseRequest(buf))
    {
        onRequest(conn,*request);
        request->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    bool closeConnection=true;
    // const std::string& connectionHeader = req.getHeader("Connection");
    // if(connectionHeader == "close"||
    //     (req.getVersion()=="HTTP/1.0" && connectionHeader != "Keep-Alive"))
    // {
    //     closeConnection=true;
    // }
    HttpResponse reponse{closeConnection};

    auto it = router_.find(req.getPath());
    if(it==router_.end()){
        if(notFoundCallback_){
            notFoundCallback_(req,&reponse);
        }
        else 
        {
            reponse.setStatusCode(HttpResponse::k404NotFound);
            reponse.setContentType("text/html; charset=utf-8");
            reponse.setBody("<html><head><title>404 Not Found</title></head>"
                         "<body><h1>404 Not Found</h1>"
                         "<p>The requested URL " + req.getPath() + " was not found on this server.</p>"
                         "</body></html>");
        }
    }
    else {
        it->second(req,&reponse);
    }

    Buffer buf;
    reponse.appendToBuffer(&buf);

    conn->send(&buf);

    if(reponse.closeConnection())
    {
        conn->shutdown();
    }
}