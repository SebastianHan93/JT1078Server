//
// Created by hc on 2020/5/19.
//
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <muduo/base/Logging.h>

#include "CConf.h"
CConf * CConf::sm_iInstance = nullptr;

CConf * CConf::GetInstance()
{
    if(sm_iInstance == nullptr)
    {
        if(sm_iInstance == nullptr)
        {
            sm_iInstance = new CConf();
        }
    }
    return sm_iInstance;
}

bool CConf::LoadConf(const char * pFileName)
{
    FILE * fp;
    fp = fopen(pFileName,"r");
    if(fp == nullptr)
    {
        LOG_ERROR << "LoadConf->打开配置文件失败";
        return false;
    }
    char gLinBuf[501];
    while(!feof(fp))
    {
        if(fgets(gLinBuf,500,fp)== nullptr)
            continue;
        //读取的一行全为0的情况
        if(gLinBuf[0]==0)
            continue;
        if(*gLinBuf==';' || *gLinBuf==' ' || *gLinBuf=='#' || *gLinBuf=='\t' ||*gLinBuf=='\n')
            continue;
        //后边若有换行，回车，空格等都截取掉
        while(strlen(gLinBuf)>0)
        {
            if(gLinBuf[strlen(gLinBuf)-1]==10 || gLinBuf[strlen(gLinBuf)-1]==13 || gLinBuf[strlen(gLinBuf)-1]==32)
            {
                gLinBuf[strlen(gLinBuf)-1] = 0;
                continue;
            }
            break;

        }
        if(gLinBuf[0]==0)
            continue;
        if(*gLinBuf=='[')
            continue;
        char * pTmp = strchr(gLinBuf,'=');
        if(pTmp != nullptr)
        {
            //获取并解析类似"PORT = 1000"配置文件
            CONF_ITEM * pItem =  new CONF_ITEM;
            memset(pItem,0,sizeof(CONF_ITEM));
            strncpy(pItem->gItemName,gLinBuf,int(pTmp-gLinBuf));
            strcpy(pItem->gItemContent,pTmp+1);
            //去掉左右两边空格
            __RTrim(pItem->gItemName);
            __LTrim(pItem->gItemName);
            __RTrim(pItem->gItemContent);
            __LTrim(pItem->gItemContent);
            LOG_DEBUG << pItem->gItemName << "-->" << pItem->gItemContent;
            m_vConfItem.push_back(pItem);
        }

    }
    fclose(fp);
    return true;


}

const char * CConf::GetString(const char * pItemName)
{
    std::vector<CONF_ITEM*>::iterator iter;
    for(iter=m_vConfItem.begin();iter!=m_vConfItem.end();iter++)
    {
        if(strcasecmp((*iter)->gItemName,pItemName)==0)
        {
            return (*iter)->gItemContent;
        }
    }
    return nullptr;
}

int CConf::GetIntDefault(const char * pItemName,const int nDef)
{
    std::vector<CONF_ITEM*>::iterator iter;
    for(iter=m_vConfItem.begin();iter!=m_vConfItem.end();iter++)
    {
        if(strcasecmp((*iter)->gItemName,pItemName)==0)
        {
            return atoi((*iter)->gItemContent);
        }
    }

    return nDef;
}

CConf::CGarbo::~CGarbo()
{
    if(CConf::sm_iInstance)
    {
        delete CConf::sm_iInstance;
        CConf::sm_iInstance = nullptr;
    }
}

CConf::CConf()
{

}

CConf::~CConf()
{
    std::vector<CONF_ITEM*>::iterator iter;
    for (iter = m_vConfItem.begin(); iter!= m_vConfItem.end(); ++iter)
    {
        delete *iter;
    }
    m_vConfItem.clear();
}

void CConf::__RTrim(char * pStr)
{
    size_t len = 0;
    if(pStr==nullptr)
    {
        return;
    }
    len = strlen(pStr);
    while(len>0 && pStr[len-1]== ' '){
        pStr[--len] = 0;
    }
}

void CConf::__LTrim(char * pStr)
{
    char * pStrTmp = pStr;
    //不以空格为开头
    if((*pStrTmp)!=' ')
        return;
    //查找不以空格为开头
    while((*pStrTmp != '\0'))
    {
        if((*pStrTmp) == ' ')
        {
            pStrTmp++;
        }else{
            break;
        }
    }
    //全是空格开头
    if((*pStrTmp) == '\0')
    {
        *pStr = '\0';
        return;
    }
    char *pStrTmpSave = pStr;
    while((*pStrTmp) != '\0')
    {
        *pStrTmpSave = *pStrTmp;
        pStrTmp++;
        pStrTmpSave++;
    }
    *pStrTmpSave = '\0';
}


