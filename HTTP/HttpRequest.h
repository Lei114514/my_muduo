#pragma once
#include <memory>
#include <map>
#include <string>
#include <cctype>
#include <algorithm>

class Buffer;

class HttpRequest
{
private:
    // Case-insensitive comparator for HTTP header keys
    struct CaseInsensitiveCompare {
        bool operator()(const std::string& a, const std::string& b) const {
            return std::lexicographical_compare(
                a.begin(), a.end(),
                b.begin(), b.end(),
                [](char ca, char cb) {
                    return std::tolower(static_cast<unsigned char>(ca)) < 
                           std::tolower(static_cast<unsigned char>(cb));
                }
            );
        }
    };

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

    enum class ChunkParseState
    {
        kExpectChunkSize,
        kExpectChunkFooter,
        kExpectChunkData,
        kExpectLastCRLF,
        kGotAllChunks,
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
    const std::map<std::string, std::string, CaseInsensitiveCompare>& getHeaders() const { return headers_; }

private:
    bool parseRequestLine(const char* begin, const char* end);
    bool parseHeader(const char* begin,const char* end);
    bool parseBody(const char* begin,const char* end);
    bool parseChunkBody(Buffer* buf);
 
    void setMethod(const char* begin,const char* end);

    void trim(const char* &begin,const char* &end);
    const char* findCRLF(const char* begin, const char* end);

    void stringToLower(std::string& line) { 
        for(char& chr:line)
        {
            chr=tolower(chr);
        }
    } 
    void stringAssign(std::string& str, const char*begin, const char* end)
    {
        str.assign(begin,end);
        stringToLower(str);
    }

    HttpRequestParseState state_;
    Method method_;
    std::string path_;
    std::string version_;
    std::string query_;

    std::map<std::string, std::string, CaseInsensitiveCompare> headers_;  //首部行
    std::string body_;  //正文內容

    size_t bytesToRead_;
    bool chunked_;
    ChunkParseState chunkState_;
    size_t chunkSize_;
};