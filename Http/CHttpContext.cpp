//
// Created by hc on 2020/5/22.
//

#include "CHttpContext.h"

CHttpContext::CHttpContext()
        : m_eState(eExpectRequestLine)
{

}

bool CHttpContext::GotAll() const
{
    return m_eState == eGotAll;
}

void CHttpContext::Reset()
{
    m_eState = eExpectRequestLine;
    CHttpRequest dummy;
    m_iRequest.Swap(dummy);
}

const CHttpRequest& CHttpContext::Request() const
{
    return m_iRequest;
}

CHttpRequest& CHttpContext::Request()
{
    return m_iRequest;
}

bool CHttpContext::ParseRequest(muduo::net::Buffer* pBuf, muduo::Timestamp iReceiveTime)
{

}

bool CHttpContext::__ProcessRequestLine(const char* pBegin, const char* pEnd)
{
    bool bSucceed = false;
    const char * pStart = pBegin;
    const char* pSpace = std::find(pStart,pEnd,' ');
    if(pSpace != pEnd && m_iRequest.SetMethod(pStart,pSpace))
    {
        pStart = pSpace+1;
        pSpace = std::find(pStart,pEnd,' ');
        if(pSpace != pEnd)
        {
            const char * pQuestion = std::find(pStart,pSpace,'?');
            if(pQuestion!=pSpace)
            {
                m_iRequest.SetPath(pStart,pQuestion);
                m_iRequest.SetQuery(pQuestion,pSpace);
            }
            else
            {
                m_iRequest.SetPath(pStart,pSpace);
            }
            pStart = pSpace + 1;
            bSucceed = pEnd - pStart == 8 && std::equal(pStart,pEnd-1,"HTTP/1.");
            if(bSucceed)
            {
                if(*(pEnd-1) == '1')
                {
                    m_iRequest.SetVersion(CHttpRequest::eHttp11);
                }
                else if(*(pEnd-1)=='0')
                {
                    m_iRequest.SetVersion(CHttpRequest::eHttp10);
                }
                else
                {
                    bSucceed = false;
                }
            }
        }
    }
    return bSucceed;
}
