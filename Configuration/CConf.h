//
// Created by hc on 2020/5/19.
//

#ifndef JT1078SERVER_CCONF_H
#define JT1078SERVER_CCONF_H


#include <muduo/base/noncopyable.h>
#include <vector>
class CConf : muduo::noncopyable
{
public:
    ~CConf();
    CConf(const CConf&) = delete;
    CConf& operator=(const CConf&) = delete;

public:
    typedef struct
    {
        char gItemName[50];
        char gItemContent[500];
    }CONF_ITEM;
    static CConf * GetInstance();
    bool LoadConf(const char * pFileName);
    const char * GetString(const char * pItemName);
    int GetIntDefault(const char * pItemName,const int nDef);
    class CGarbo
    {
    public:
        CGarbo() = default;
        ~CGarbo();
    };
private:
    CConf();

private:
    void __RTrim(char * pStr);
    void __LTrim(char * pStr);


private:
    static CConf * sm_iInstance;
    std::vector<CONF_ITEM*> m_vConfItem;
    CGarbo m_iGarbo;
};


#endif //JT1078SERVER_CCONF_H
