//
// Created by hc on 2020/5/22.
//

#ifndef JT1078SERVER_CHTTPREQUEST_H
#define JT1078SERVER_CHTTPREQUEST_H


#include <muduo/base/copyable.h>
#include <string>
#include <map>
#include "../../../muduo/build/release-install-cpp11/include/muduo/base/Timestamp.h"

class CHttpContext;
class CHttpRequest: public muduo::copyable
{
public:
    enum METHOD
    {
        eInvalid,eGet,ePost,eHead,ePut,eDelete
    };
    enum VERSION
    {
        eUnknown,eHttp10,eHttp11
    };
public:
    CHttpRequest();
public:
    void SetVersion(VERSION v);
    VERSION  GetVersion() const;
    bool SetMethod(const char * pStart,const char * pEnd);
    METHOD GetMethod() const;
    const char * GetMethodString() const;
    void SetPath(const char* pStart,const char*pEnd);
    const std::string & GetPath() const;
    void SetQuery(const char* pStart,const char* pEnd);
    const std::string & GetQuery() const;
    void SetReceiveTime(muduo::Timestamp timestamp);
    muduo::Timestamp GetReceiveTime()const ;
    void AddHeader(const char* pStart,const char* pColon,const char * pEnd);
    std::string GetHeader(const std::string & sField) const ;
    const std::map<std::string,std::string>& GetHeaders() const;

    void Swap(CHttpRequest& that);

private:
    METHOD m_eMethod;
    VERSION m_eVersion;
    std::string m_sPath;
    std::string m_sQuery;
    muduo::Timestamp m_iReceiveTime;
    std::map<std::string,std::string> m_mHeaders;
};


#endif //JT1078SERVER_CHTTPREQUEST_H
