//
// Created by hc on 2020/5/19.
//

#ifndef JT1078SERVER_CURLMANAGER_H
#define JT1078SERVER_CURLMANAGER_H

#include <map>
#include <muduo/base/Mutex.h>
#include <muduo/net/TcpConnection.h>

class CURLManager
{
public:
//    CUrlManager(std::map<std::string,pTcpConnectionSt>& urlMap);
    typedef std::weak_ptr<muduo::net::TcpConnection> WEAK_TCP_CONNECTION_PTR;
    CURLManager();
    ~CURLManager();
    void Erase(const std::string &key);
    bool Insert(const std::pair<std::string,WEAK_TCP_CONNECTION_PTR> &value);
    std::pair<std::string,WEAK_TCP_CONNECTION_PTR> Find(const std::string &key);
    bool IsInUrlMap(const std::string &key);
private:
    void __Clear();
private:
    std::map<std::string,WEAK_TCP_CONNECTION_PTR> m_mURLMap;
    muduo::MutexLock m_iLock;
};


#endif //JT1078SERVER_CURLMANAGER_H
