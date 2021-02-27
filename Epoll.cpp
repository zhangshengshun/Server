#include <string.h>
#include <errno.h>
#include<iostream>

#include"Epoll.h"
#include"EpollEvent.h"
#include"server.h"
#include"Error.h"
#include"client.h"

#define EPOLL_TIMEOUT_LEN 5

using std::cout;

Epoll::Epoll():m_epollEvents(nullptr),m_epollFd(-1){
    
}

Epoll::~Epoll(void){
    if(m_epollEvents!=nullptr){
        delete[] m_epollEvents;
        m_epollEvents=nullptr;
    }
}

int Epoll::getEpollFd()const{
    return m_epollFd;
}

int Epoll::initialize(int fdsize){
    m_eventSize=fdsize;
    m_epollFd=epoll_create1(::EPOLL_CLOEXEC);

    if(m_epollFd<0){
        cout<<"Epoll:initialize error"<<endl;
        return FAILED;
    }
    m_epollEvents=new epoll_event[fdsize];
    memset(m_epollEvents, 0, sizeof( epoll_event )*fdsize);
    return SUCCESSFUL;
}

int Epoll::doEvent(EpollEvent *ptr,int fd,int op,unsigned int events){
    struct epoll_event ev;
    memset( &ev, 0, sizeof( struct epoll_event ) );
    ev.events=events;
    ev.data.ptr=ptr;
    if(epoll_ctl(m_epollFd, op, fd,&ev)<0){
        cout<<"Epoll:doEvent epoll_ctl() error "<<strerror(errno)<<endl;
        return FAILED;
    }
    return SUCCESSFUL;
}

