/*
 * @Descripttion: 
 * @version: 
 * @Author: sueRimn
 * @Date: 2021-01-30 11:04:42
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-26 14:02:37
 */

#ifndef __EPOLL__
#define __EPOLL__

#include <sys/epoll.h>
#include"EpollEvent.h"
using namespace std;

class Server;
class client;
class Epoll
{
public:
    Epoll();
    ~Epoll( void );
    int getEpollFd()const;
    int initialize( int );
    int doEvent(EpollEvent*ptr,int fd, int op, unsigned int events );
    struct epoll_event*   m_epollEvents;
    int                   m_epollFd;
    int                   m_eventSize;
};

#endif