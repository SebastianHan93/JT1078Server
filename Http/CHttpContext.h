//
// Created by hc on 2020/5/22.
//

#ifndef JT1078SERVER_CHTTPCONTEXT_H
#define JT1078SERVER_CHTTPCONTEXT_H
#include "muduo/base/copyable.h"
#include "muduo/base/copyable.h"
#include "muduo/base/Timestamp.h"
#include "muduo/base/Types.h"
#include "CHttpRequest.h"
#include <muduo/net/Buffer.h>


class CHttpContext {
public:
    enum HTTP_REQUEST_PARSE_STATE
    {
        eExpectRequestLine,
        eExpectHeaders,
        eExpectBody,
        eGotAll,
    };

    CHttpContext();

    // default copy-ctor, dtor and assignment are fine

    // return false if any error
    bool ParseRequest(muduo::net::Buffer* pBuf, muduo::Timestamp iReceiveTime);

    bool GotAll() const;

    void Reset();

    const CHttpRequest& Request() const;

    CHttpRequest& Request();

private:
    bool __ProcessRequestLine(const char* pBegin, const char* pEnd);

    HTTP_REQUEST_PARSE_STATE m_eState;
    CHttpRequest m_iRequest;
};


#endif //JT1078SERVER_CHTTPCONTEXT_H
