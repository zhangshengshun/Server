/*
 * @Descripttion: 
 * @version: 
 * @Author: sueRimn
 * @Date: 2021-02-26 14:02:20
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-26 15:52:36
 */
#include"TCPIOServer.h"
#include"Epoll.h"
#include"EpollEvent.h"
#include"TCPConnection.h"
#include<thread>

using std::cout;
using std::endl;

void TCPIOServer::Init(){
    while(1){
        if(stop){
            break;
        }
        this->runInIOServer();
    }
}

void TCPIOServer::runInIOServer(){
    int nfds=0;
    EpollEvent *event=nullptr;

    nfds=::epoll_wait(this->epollPtr->m_epollFd, this->epollPtr->m_epollEvents, this->epollPtr->m_eventSize, -1);
    for(int i=0;i<nfds;i++){
        event=(EpollEvent *)this->epollPtr->m_epollEvents[i].data.ptr;
        TCPConnection* connect=this->connectManager[event->m_epollEvent.m_id];
        if(connect==nullptr){
            cout<<"connect == NULL"<<endl;
            continue;
        }  
        else if(this->epollPtr->m_epollEvents[i].events & EPOLLOUT){
            if(connect->sendData()<=0){
                connect->event.closeWevent();
            }
            continue;
        }
        else if(this->epollPtr->m_epollEvents[i].events & EPOLLIN){
            if(connect->readData()<0){   
            }
            continue;
        }
        else if(this->epollPtr->m_epollEvents[i].events &EPOLLERR || this->epollPtr->m_epollEvents[i].events & EPOLLHUP){
            cout<<"EPOLLERR"<<endl;
            connect->releaseSendBuffer();
            if(connect->m_InReq.ioBuf!=nullptr){
                delete[] connect->m_InReq.ioBuf;
                connect->m_InReq.ioBuf=nullptr;
            }
            memset(&connect->m_InReq.m_msgHeader,0,HEADER_SIZE);
            connect->server->connectManager.erase(event->m_epollEvent.m_id);
            continue;
        }
    }
}

void TCPIOServer::startHandle(){
    std::thread t(&TCPIOServer::Init,this);
    t.detach(); 
}