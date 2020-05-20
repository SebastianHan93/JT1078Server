//
// Created by hc on 2020/5/19.
//

#include "CURLManager.h"

CURLManager::CURLManager()
{

}

CURLManager::~CURLManager()
{
    __Clear();
}

void CURLManager::Erase(const std::string &key)
{
    muduo::MutexLockGuard iLockGuard(m_iLock);
}

bool CURLManager::Insert(const std::pair<std::string,WEAK_TCP_CONNECTION_PTR> &value)
{
    muduo::MutexLockGuard iLockGuard(m_iLock);
//    std::pair<std::map<std::string,CURLManager::WEAK_TCP_CONNECTION_PTR>::iterator,bool>
    auto ret =  m_mURLMap.insert(value);
    return ret.second;
}

std::pair<std::string,CURLManager::WEAK_TCP_CONNECTION_PTR> CURLManager::Find(const std::string &key)
{
    muduo::MutexLockGuard iLockGuard(m_iLock);
    auto it = m_mURLMap.find(key);
    if(it!=m_mURLMap.end())
        return std::pair<std::string,WEAK_TCP_CONNECTION_PTR>(it->first,it->second);
    return std::pair<std::string,WEAK_TCP_CONNECTION_PTR>("",WEAK_TCP_CONNECTION_PTR());
}

bool CURLManager::IsInUrlMap(const std::string &key)
{
    muduo::MutexLockGuard iLockGuard(m_iLock);
    auto it = m_mURLMap.find(key);
    return it != m_mURLMap.end();
}

void CURLManager::__Clear()
{

    if(!m_mURLMap.empty())
    {
        {
            muduo::MutexLockGuard iLockGuard(m_iLock);
            m_mURLMap.clear();
        }
    }
}