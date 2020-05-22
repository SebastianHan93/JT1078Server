//
// Created by hc on 2020/5/22.
//

#include "CHttpResponse.h"

CHttpResponse::CHttpResponse(bool bClose)
    :m_eStatusCode(eUnkown),
    m_bCloseConnection(bClose)
{

}

void CHttpResponse::SetStatusCode(HTTP_STATUS_CODE eCode)
{
    m_eStatusCode = eCode;
}

void CHttpResponse::SetStatusMessage(const std::string& sMessage)
{
    m_sStatusMessage = sMessage;
}

void CHttpResponse::SetCloseConnection(bool bOn)
{
    m_bCloseConnection = bOn;
}

bool CHttpResponse::GetCloseConnection() const
{
    return m_bCloseConnection;
}

void CHttpResponse::SetContentType(const std::string & sContentType)
{
    AddHeader("Content-Type",sContentType);
}

void CHttpResponse::AddHeader(const std::string & sKey,const std::string& sValue)
{
    m_mHeaders[sKey] = sValue;
}

void CHttpResponse::SetBody(const std::string & sBody)
{
    m_sBody = sBody;
}

void CHttpResponse::AppendToBuffer(muduo::net::Buffer* pOutPut) const
{
    char gBuf[32];
    snprintf(gBuf, sizeof(gBuf),"HTTP/1.1 %d ", m_eStatusCode);
    pOutPut->append(gBuf);
    pOutPut->append(m_sStatusMessage);
    pOutPut->append("\r\n");
    if(m_bCloseConnection)
    {
        pOutPut->append("Connection: close\r\n");
    }
    else
    {
        snprintf(gBuf, sizeof gBuf, "Content-Length: %zd\r\n", m_sBody.size());
        pOutPut->append(gBuf);
        pOutPut->append("Connection: Keep-Alive\r\n");
    }
    for (const auto& header:m_mHeaders)
    {
        pOutPut->append(header.first);
        pOutPut->append(": ");
        pOutPut->append(header.second);
        pOutPut->append("\r\n");
    }
    pOutPut->append("\r\n");
    pOutPut->append(m_sBody);
}