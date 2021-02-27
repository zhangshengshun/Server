/*
 * @Descripttion: 
 * @version: 
 * @Author: sueRimn
 * @Date: 2021-02-26 13:39:29
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-26 15:57:53
 */
#include"TCPServer.h"
#include"Error.h"
#include"Epoll.h"
#include"TCPIOServer.h"
#include"TCPConnection.h"
#include"EpollEvent.h"

#include<iostream>
#include <sys/epoll.h>
#include<map>
#include<list>
#include<netinet/in.h>
#include<sys/uio.h>
#include <unistd.h>
#include<cstring>

TCPServer::TCPServer():fdNum(0),IOServerNum(0){

}

void TCPServer::Init(){
    this->nListenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == nListenSocket)
    {
        std::cout << "socket error" << std::endl;
    }
    sockaddr_in ServerAddress;
    memset(&ServerAddress, 0, sizeof(sockaddr_in));
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerAddress.sin_port = htons(4000);
    if(::bind(nListenSocket, (sockaddr *)&ServerAddress, sizeof(sockaddr_in)) == -1)
    {
        std::cout << "bind error" << std::endl;
        ::close(nListenSocket);
    }
}

int TCPServer::listen(){
    if(::listen(nListenSocket,23)<0){
        cerr<<"TCPSocket::listen::listen"<<std::endl;
        return FAILED;
    }
    event_st epollevent;
    epollevent.fd=nListenSocket;
    
    //创建epoll
    this->epollPtr=new Epoll();
    this->epollPtr->initialize(1024);

    this->connection=new TCPConnection(this->epollPtr);
    this->connection->event.m_epollEvent=epollevent;

    this->connection->event.registerREvent();
    this->connection->sockfd=nListenSocket;

    this->connection->disableLinger();
    this->connection->enableReuseaddr();
    this->connection->disableNagle();
    
    return SUCCESSFUL;
}

int TCPServer::distributeConnection(int fd,int num,bool IoStart){
    //得到IOServer
    TCPIOServer* server=this->IOServer[num];
    if(server==nullptr){
        return FAILED;
    }
    //创建Epoll
    if(IoStart){
        Epoll * epollPtr_=new Epoll();
        epollPtr_->initialize(1024);
        server->epollPtr=epollPtr_;
    }

    TCPConnection* connection=new TCPConnection(server,server->epollPtr);

    event_st epollevent;
    epollevent.fd=fd;
    epollevent.m_id=server->startID;

    connection->event.m_epollEvent=epollevent;
    connection->sockfd=fd;

    //cout<<"startID:"<<server->startID<<endl;
    
    server->connectManager[server->startID]=connection;
    server->startID++;

    connection->disableLinger();
    connection->enableReuseaddr();
    connection->disableNagle();


    connection->event.registerREvent();
    connection->sendPackage(epollevent.m_id);
    
    //得修改
    if(IoStart){
        server->startHandle();
    }
    return SUCCESSFUL;
}

int TCPServer::accept(){
    sockaddr_in ClientAddress;
    socklen_t LengthOfClientAddress = sizeof(sockaddr_in);
    int clientfd = ::accept(nListenSocket, (sockaddr *)&ClientAddress, &LengthOfClientAddress);
    if(-1 == clientfd)
    {
        std::cout << "accept error" << std::endl;
        ::close(nListenSocket);
        return FAILED;
    }
    
    if(IOServerNum<maxIOServerNum){
        TCPIOServer *server=new TCPIOServer();
        //server->IoID=fdNum%maxIOServerNum;
        this->IOServer[fdNum%maxIOServerNum]=server;
        this->distributeConnection(clientfd,fdNum%maxIOServerNum,true);
        
        fdNum++;
        IOServerNum++;
    }
    else{
        this->distributeConnection(clientfd,fdNum%maxIOServerNum,false);
        fdNum++;
    }
    return SUCCESSFUL;
}

int TCPServer::runInServer(){
    int nfds=0;
    EpollEvent *event=nullptr;
    while(1){
        nfds=::epoll_wait(this->epollPtr->m_epollFd, this->epollPtr->m_epollEvents, this->epollPtr->m_eventSize, -1);
        for(int i=0;i<nfds;i++){
            event=(EpollEvent *)this->epollPtr->m_epollEvents[i].data.ptr;
            if(event->m_epollEvent.fd==this->nListenSocket){
                this->accept();
                continue;
            }
        }
    }
}

void TCPServer::start(){
    this->Init();
    this->listen();
    
    this->runInServer();
}
