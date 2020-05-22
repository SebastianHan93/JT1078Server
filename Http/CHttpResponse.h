//
// Created by hc on 2020/5/22.
//

#ifndef JT1078SERVER_CHTTPRESPONSE_H
#define JT1078SERVER_CHTTPRESPONSE_H


#include <muduo/base/copyable.h>
#include <string>
#include <muduo/net/Buffer.h>
#include <map>

class CHttpResponse : public muduo::copyable
{
public:
    enum HTTP_STATUS_CODE
    {
        eUnkown,
        e2000 = 200,
        e301MovedPermanently = 301,
        e400BadRequest = 400,
        e404NotFound = 404,
    };
public:
    explicit CHttpResponse(bool bClose);

public:
    void SetStatusCode(HTTP_STATUS_CODE eCode);
    void SetStatusMessage(const std::string& sMessage);
    void SetCloseConnection(bool bOn);
    bool GetCloseConnection() const;
    void SetContentType(const std::string & sContentType);
    void AddHeader(const std::string & sKey,const std::string& sValue);
    void SetBody(const std::string & sBody);
    void AppendToBuffer(muduo::net::Buffer* pOutPut) const;

private:
    std::map<std::string,std::string> m_mHeaders;
    HTTP_STATUS_CODE m_eStatusCode;
    std::string m_sStatusMessage;
    bool m_bCloseConnection;
    std::string m_sBody;

};


#endif //JT1078SERVER_CHTTPRESPONSE_H
