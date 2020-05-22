//
// Created by hc on 2020/5/22.
//

#include "CHttpRequest.h"
#include "CHttpContext.h"

using std::string;
CHttpRequest::CHttpRequest()
    :m_eMethod(eInvalid),
    m_eVersion(eUnknown)
{

}
void CHttpRequest::SetVersion(VERSION v)
{
    m_eVersion = v;
}

CHttpRequest::VERSION  CHttpRequest::GetVersion() const
{
    return m_eVersion;
}

bool CHttpRequest::SetMethod(const char * pStart,const char * pEnd)
{
    assert(m_eMethod == eInvalid);
    string sMethod(pStart,pEnd);

    if(sMethod=="GET")
    {
        m_eMethod = eGet;
    }
    else if (sMethod == "POST")
    {
        m_eMethod = ePost;
    }
    else if (sMethod == "HEAD")
    {
        m_eMethod = eHead;
    }
    else if (sMethod == "PUT")
    {
        m_eMethod = ePut;
    }
    else if (sMethod == "DELETE")
    {
        m_eMethod = eDelete;
    }
    else
    {
        m_eMethod = eInvalid;
    }
    return m_eMethod != eInvalid;

}

CHttpRequest::METHOD CHttpRequest::GetMethod() const
{
    return m_eMethod;
}

const char * CHttpRequest::GetMethodString() const
{
    const char * pResult = "UNKNOWN";
    switch (m_eMethod)
    {
        case eGet:
            pResult = "GET";
        case ePost:
            pResult = "POST";
        case eHead:
            pResult = "HEAD";
        case ePut:
            pResult = "PUT";
        case eDelete:
            pResult = "DELETE";
        default:
            break;
    }
    return pResult;
}

void CHttpRequest::SetPath(const char* pStart,const char*pEnd)
{
    m_sPath.assign(pStart,pEnd);
}

const std::string & CHttpRequest::GetPath() const
{
    return m_sPath;
}

void CHttpRequest::SetQuery(const char* pStart,const char* pEnd)
{
    m_sQuery.assign(pStart,pEnd);
}

const std::string & CHttpRequest::GetQuery() const
{
    return m_sQuery;
}

void CHttpRequest::SetReceiveTime(muduo::Timestamp timestamp)
{
    m_iReceiveTime =timestamp;
}

muduo::Timestamp CHttpRequest::GetReceiveTime()const
{
    return m_iReceiveTime;
}

void CHttpRequest::AddHeader(const char* pStart,const char* pColon,const char * pEnd)
{
    string sField(pStart,pColon);
    ++ pColon;
    while(pColon<pEnd && isspace(*pColon))
    {
        ++pColon;
    }
    string sValue(pColon,pEnd);
    while(!sValue.empty()&&isspace(sValue[sValue.size()-1]))
    {
        sValue.resize(sValue.size()-1);
    }
    m_mHeaders[sField] = sValue;
}

std::string CHttpRequest::GetHeader(const std::string & sField) const
{
    string sResult;
    std::map<string,string>::const_iterator it = m_mHeaders.find(sField);
    if(it != m_mHeaders.end())
    {
        sResult = it->second;
    }
    return sResult;
}

const std::map<std::string,std::string>& CHttpRequest::GetHeaders() const
{
    return m_mHeaders;
}

void CHttpRequest::Swap(CHttpRequest& that)
{
    std::swap(m_eMethod, that.m_eMethod);
    std::swap(m_eVersion, that.m_eVersion);
    m_sPath.swap(that.m_sPath);
    m_sQuery.swap(that.m_sQuery);
    m_iReceiveTime.swap(that.m_iReceiveTime);
    m_mHeaders.swap(that.m_mHeaders);
}