//
// Created by hc on 2020/5/19.
//

#include <unistd.h>
#include <cerrno>
#include <sys/stat.h>
#include <fcntl.h>
#include <muduo/base/Logging.h>
#include "Daemon.h"

int Daemon(const char * pDirectory)
{
    //(1)fork子进程
    switch (fork())
    {
        case -1:
            LOG_ERROR << "Daemon->fork子进程失败" ;
            return -1;
        case 0:
            break;
        default:
            //父进程以往直接退出exit,现在希望回到主流程去释放一些资源
            return 1;
    }

    //(2)脱离终端，终端关闭
    if (setsid() == -1)
    {
        LOG_ERROR << "Daemon->setsid 错误" ;
        return -1;
    }
    //设定工作目录;
    if(pDirectory== nullptr)
    {
        chdir("/");
    }
    else
    {
        chdir(pDirectory);
    }
    errno = 0;
    //(4)umask设置为0
    umask(0);
    //(5)打开黑洞设备,重定向
    int fd = open("/dev/null", O_RDWR);
    if (fd == -1)
    {
        LOG_ERROR << "Daemon->open 错误" ;
        return -1;
    }
    //关闭STDIN_FILENO,让/dev/null成为标准输入
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        LOG_ERROR << "Daemon->dup2 错误" ;
        return -1;
    }
    //关闭STDIN_FILENO，让/dev/null成为标准输出
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        LOG_ERROR << "Daemon->dup2 错误" ;
        return -1;
    }
    //关闭STDERR_FILENO，让/dev/null成为标准输出

    //释放资源
    if (fd > STDERR_FILENO)
    {
        if (close(fd) == -1)
        {
            LOG_ERROR << "Daemon->close 错误" ;
            return -1;
        }
    }
    return 0;
}
