//
// Created by hc on 2020/5/19.
//

#include <muduo/base/Logging.h>
#include "CRedisCluster.h"
#include "../Configuration/CConf.h"

CRedisCluster::CRedisCluster()
    :m_pClusterAddr(CConf::GetInstance()->GetString("RedisClusterAddr")),
     m_pCtx(redisClusterContextInit())
{
    __Connect();
}

CRedisCluster::~CRedisCluster()
{
    if(this->m_pCtx!=nullptr)
    {
        redisClusterFree(this->m_pCtx);
    }
    m_pClusterAddr = nullptr;
}


void CRedisCluster::__Connect()
{
//
//    m_pCtx = redisClusterContextInit();
//    redisClusterSetOptionAddNodes(m_pCtx,m_pClusterAddr);
    redisClusterConnect2(m_pCtx);
    if (m_pCtx != nullptr && m_pCtx->err)
    {
        LOG_ERROR << "连接redis服务器失败["<<m_pClusterAddr<<"]"<<"--Error[" << m_pCtx->errstr<<"]";
        exit(-1);
    }
}

std::string CRedisCluster::Get(const std::string & sKey )
{
    muduo::MutexLockGuard iLockGuard(m_iLock);
    m_pReply = (redisReply*) redisClusterCommand(m_pCtx,"GET %s", const_cast<const char *>(sKey.data()));
    if(m_pReply == nullptr|| m_pReply->str== nullptr)
    {
        LOG_ERROR << "获取["<<sKey<<"]"<<"失败";
        return std::string("");
    }
    std::string sRet = m_pReply->str;
    freeReplyObject(m_pReply);
    return sRet;
}

std::string CRedisCluster::Hget(const std::string & sKey,const std::string & sField)
{
    muduo::MutexLockGuard iLockGuard(m_iLock);
    m_pReply = (redisReply*) redisClusterCommand(m_pCtx,"HGET %s %s", const_cast<const char *>(sKey.data()),const_cast<const char *>(sField.data()));
    if(m_pReply == nullptr || m_pReply->str== nullptr)
    {
        LOG_ERROR << "获取["<<sKey<<"]"<<"失败";
        return std::string("");
    }
    std::string sRet = m_pReply->str;
    freeReplyObject(m_pReply);
    return sRet;
}

bool CRedisCluster::Set(const std::string & sKey, const std::string & sValue)
{
    muduo::MutexLockGuard iLockGuard(m_iLock);
    m_pReply = (redisReply*) redisClusterCommand(m_pCtx,"HSET %s %s %s",
                                                 const_cast<const char *>(sKey.data()),
                                                 const_cast<const char *>(sValue.data()));
    if(m_pReply == nullptr)
    {
        LOG_ERROR << "设置["<<sValue<<"]"<<"失败";
        return false;
    }
    freeReplyObject(m_pReply);
    return true;
}

bool CRedisCluster::Hset(const std::string & sKey, const std::string & sField,const std::string & sValue)
{
    muduo::MutexLockGuard iLockGuard(m_iLock);
    m_pReply = (redisReply*) redisClusterCommand(m_pCtx,"HSET %s %s %s",
            const_cast<const char *>(sKey.data()),
            const_cast<const char *>(sField.data()),
            const_cast<const char *>(sValue.data()));
    if(m_pReply == nullptr)
    {
        LOG_ERROR << "设置["<<sValue<<"]"<<"失败";
        return false;
    }
    freeReplyObject(m_pReply);
    return true;
}

