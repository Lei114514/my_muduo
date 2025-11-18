#pragma once
#include <memory>
#include <map>
#include <string>

class Buffer;

class HttpRequest
{
public:
    enum class Method
    {
        kInvalid,
        kGet,
        kPost,
        kHead,
        kPut,
        kDelete,
    };

    enum class HttpRequestParseState
    {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

    HttpRequest() : state_(HttpRequestParseState::kExpectRequestLine), method_(Method::kInvalid) { }
    ~HttpRequest() = default;

    bool parseRequest(Buffer* buf);
    void reset();

    bool isGotAll() const { return state_==HttpRequestParseState::kGotAll; }
    Method getMethod() const { return method_; }
    const std::string& getPath() const { return path_; }
    const std::string& getVersion() const { return version_; }
    const std::string& getQuery() const { return query_; }
    const std::string& getBody() const { return body_; }
    std::string getHeader(const std::string& field) const;
    const std::map<std::string, std::string>& getHeaders() const { return headers_; }

private:
    bool parseRequestLine(const char* begin, const char* end);
    void setMethod(const char* begin,const char* end);

    HttpRequestParseState state_;
    Method method_;
    std::string path_;
    std::string version_;
    std::string query_;

    std::map<std::string,std::string> headers_;  //首部行
    std::string body_;  //正文內容
};