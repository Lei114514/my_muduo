#include <algorithm>

#include "HttpRequest.h"
#include "../Net/Buffer.h"
#include "Logger.h"

bool HttpRequest::parseRequest(Buffer* buf)
{
    bool keepParsing=true;
    while(keepParsing)
    {
        switch (state_)
        {
            case HttpRequestParseState::kExpectRequestLine :
            {
                LOG_DEBUG("Request Line");
                const char* crlf=findCRLF(buf->peek(),buf->peek()+buf->readableBytes());
                if(crlf!=nullptr)
                {
                    LOG_DEBUG("find crlf");
                    if(!parseRequestLine(buf->peek(),crlf)) 
                    {
                        LOG_ERROR("parseRequestLine error");
                        return false;
                    }
                    buf->retrieve(crlf+2-buf->peek());
                    state_=HttpRequestParseState::kExpectHeaders;
                }
                else 
                {
                    LOG_DEBUG("can't find crlf");
                    keepParsing=false;
                }
                break;
            }

            case HttpRequestParseState::kExpectHeaders : 
            {
                LOG_DEBUG("Header");
                const char* begin=buf->peek();
                const char* crlf = findCRLF(begin,begin+buf->readableBytes());
                if(crlf==nullptr)
                {
                    keepParsing=false;
                    break;
                }

                if(begin!=crlf)
                {
                    if(!parseHeader(begin,crlf)) 
                    { 
                        LOG_ERROR("parseHeader error");
                        return false; 
                    }
                    buf->retrieve(crlf+2-begin);
                }
                else 
                {
                    buf->retrieve(2);
                    std::string transferEncoding = getHeader("Transfer-Encoding");
                    std::string contentLength = getHeader("Content-Length");
                    if(transferEncoding == "chunked")
                    {
                        chunked_=true;
                        state_=HttpRequestParseState::kExpectBody;
                    } 
                    else if(contentLength != "")
                    {
                        try{
                            bytesToRead_ = std::stoi(contentLength);
                            if(bytesToRead_ == 0)
                            {
                                state_=HttpRequestParseState::kGotAll;
                            }
                            else
                            {
                                state_=HttpRequestParseState::kExpectBody;
                            }
                        }
                        catch(const std::exception& )
                        {
                            LOG_ERROR("Content-Length value is not integer");
                            return false;
                        }
                    }
                    else 
                    {
                        state_ = HttpRequestParseState::kGotAll;
                    }
                }
                break;
            }

            case HttpRequestParseState::kExpectBody :
            {
                LOG_DEBUG("Body");
                if(chunked_)
                {
                    if(parseChunkBody(buf)) 
                    { 
                        state_=HttpRequestParseState::kGotAll;
                    }
                    break;
                }
                else 
                {
                    if(buf->readableBytes()<bytesToRead_)
                    {
                        keepParsing=false;
                        break;
                    }
                    if(!parseBody(buf->peek(),buf->peek()+buf->readableBytes())) 
                    { 
                        LOG_ERROR("parseBody error");
                        return false; 
                    }
                    buf->retrieve(bytesToRead_);
                    state_=HttpRequestParseState::kGotAll;
                }
                break;
            }

            case HttpRequestParseState::kGotAll:
            {
                LOG_DEBUG("Got All");
                keepParsing=false;
                break;
            }
            
            default:
                break;
        }
    }
    return isGotAll();
}


bool HttpRequest::parseRequestLine(const char* begin, const char* end)
{
    //method
    const char* start=begin;
    const char* space1=std::find(begin,end,' ');
    if(space1==end)
    {
        LOG_ERROR("invalid request line format");
        return false;
    }
    setMethod(start,space1);

    //path
    start=space1+1;
    while(start!=end && *start==' '){
        ++start;
    }
    if(start==end)
    {
        LOG_ERROR("invalid request line format");
        return false;
    }
    const char* space2=std::find(start,end,' ');
    if(space2==end)
    {
        LOG_ERROR("invalid request line format");
        return false;
    }
    const char* question=std::find(start,space2,'?');
    if(question==space2){
        stringAssign(path_,start,space2);
        //path_.assign(start,space2);
    }
    else {
        stringAssign(path_,start,question);
        stringAssign(query_,question+1,space2);
        //path_.assign(start,question);
        //query_.assign(question+1,space2);
    }

    //version
    start=space2+1;
    while(start!=end && *start==' '){
        ++start;
    }
    if(start==end)
    {
        LOG_ERROR("invalid request line format");
        return false;
    }
    stringAssign(version_,start,end);
    //version_.assign(start,end);

    return true;
}

bool HttpRequest::parseHeader(const char* begin,const char* end)
{
    const char* colon=std::find(begin,end,':');
    if(colon==end)
    {
        LOG_ERROR("invalid header line format");
        return false;
    }

    const char* key_begin = begin;
    const char* key_end = colon;
    trim(key_begin,key_end);
    if(key_begin==key_end)
    {
        LOG_ERROR("header key empty");
        return false;
    }

    const char* value_begin = colon + 1;
    const char* value_end = end;
    trim(value_begin,value_end);

    std::string key,value;
    stringAssign(key,key_begin,key_end);
    stringAssign(value,value_begin,value_end);
    headers_[key]=value;

    return true;
}

bool HttpRequest::parseBody(const char* begin,const char* end)
{
    body_.assign(begin,begin+bytesToRead_);
    return true;
}

bool HttpRequest::parseChunkBody(Buffer* buf)
{
    bool keepParseChunk=true;
    while(keepParseChunk)
    {
        switch (chunkState_)
        {
            case ChunkParseState::kExpectChunkSize :
            {
                const char* begin=buf->peek();
                const char* crlf=findCRLF(begin,begin+buf->readableBytes());
                if(crlf==nullptr) 
                {
                    keepParseChunk=false;
                    break;
                }

                try{
                    chunkSize_=std::stoul(std::string(begin,crlf),nullptr,16);
                }catch(const std::exception& e){
                    LOG_ERROR("ChunkSize is not integer");
                    return false;
                }
                
                buf->retrieve(crlf+2-begin);
                if(chunkSize_==0)
                {
                    chunkState_=ChunkParseState::kExpectLastCRLF;
                }
                else 
                {
                    chunkState_=ChunkParseState::kExpectChunkData;
                }
                break;
            }

            case ChunkParseState::kExpectChunkData :
            {
                size_t readLen=std::min(chunkSize_,buf->readableBytes());
                body_.append(buf->peek(),readLen);

                buf->retrieve(readLen);
                chunkSize_-=readLen;

                if(chunkSize_==0)
                {
                    chunkState_=ChunkParseState::kExpectChunkFooter;
                }
                else 
                {
                    keepParseChunk=false;
                }
                break;
            }

            case ChunkParseState::kExpectChunkFooter:
            {
                if(buf->readableBytes()<2)
                {
                    keepParseChunk=false;
                    break;
                }
                if(*buf->peek()!='\r' || *(buf->peek()+1)!='\n')
                {
                    return false;
                }
                buf->retrieve(2);
                chunkState_=ChunkParseState::kExpectChunkSize;
                break;
            }

            case ChunkParseState::kExpectLastCRLF :
            {
                if(buf->readableBytes()<2)
                {
                    keepParseChunk=false;
                    break;
                }
                if(*buf->peek()!='\r' || *(buf->peek()+1)!='\n')
                {
                    return false;
                }
                buf->retrieve(2);
                chunkState_=ChunkParseState::kGotAllChunks;
                break;
            }

            case ChunkParseState::kGotAllChunks :
            {
                keepParseChunk=false;
                break;
            }
        }
    }
    return chunkState_==ChunkParseState::kGotAllChunks;
}

std::string HttpRequest::getHeader(const std::string& field) const 
{
    auto it = headers_.find(field);
    return it!=headers_.end()?it->second:"";
}

void HttpRequest::setMethod(const char* begin,const char* end)
{
    std::string data(begin,end);
    stringToLower(data);
    if(data=="get"){
        method_=Method::kGet;
    }
    else if(data=="post"){
        method_=Method::kPost;
    }
    else if(data=="head"){
        method_=Method::kHead;
    }
    else if(data=="put"){
        method_=Method::kPut;
    }
    else if(data=="delete"){
        method_=Method::kDelete;
    }
    else {
        method_=Method::kInvalid;
        LOG_ERROR("Http Method invalid");
    }
}


void HttpRequest::reset()
{
    state_=HttpRequestParseState::kExpectRequestLine;
    method_=Method::kInvalid;
    path_.clear();
    version_.clear();
    query_.clear();
    headers_.clear();
    body_.clear();
    bytesToRead_=0;
    chunked_=false;
    chunkState_=ChunkParseState::kExpectChunkSize;
    chunkSize_=0;
}

void HttpRequest::trim(const char* &begin,const char* &end)
{
    while( begin<end && isspace(*begin) ){
        ++begin;
    }
    while( begin<end && isspace( *(end-1) ) ){
        --end;
    }
}

const char* HttpRequest::findCRLF(const char* begin, const char* end)
{
    for(const char* index=begin;index+1<end;++index)
    {
        if(*index=='\r' && *(index+1)=='\n' ) {
            return index;
        }
    }
    return nullptr;
}
