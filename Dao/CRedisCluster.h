//
// Created by hc on 2020/5/19.
//

#ifndef JT1078SERVER_CREDISCLUSTER_H
#define JT1078SERVER_CREDISCLUSTER_H

#include <rediscluster/rediscluster.h>
#include <muduo/base/noncopyable.h>
#include <string>
#include <muduo/base/Mutex.h>

class CRedisCluster : muduo::noncopyable
{
public:
    CRedisCluster();
    ~CRedisCluster();

    std::string Get(const std::string & sKey );
    std::string Hget(const std::string & sKey,const std::string & sField);
    bool Set(const std::string & sKey, const std::string & sValue);
    bool Hset(const std::string & sKey, const std::string & sField,const std::string & sValue);

private:
    void __Connect();
private:
    redisReply* m_pReply;
    redisClusterContext * m_pCtx;
    muduo::MutexLock m_iLock;
    const char * m_pClusterAddr;
};


#endif //JT1078SERVER_CREDISCLUSTER_H
