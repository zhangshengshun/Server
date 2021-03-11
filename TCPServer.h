/*
 * @Descripttion: 
 * @version: 
 * @Author: sueRimn
 * @Date: 2021-02-26 13:39:18
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-26 15:53:53
 */
#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include<map>

const int maxIOServerNum=3;

class TCPIOServer;
class TCPConnection;
class Epoll;
class TCPServer{
    public:
    TCPServer();
    
    void Init();
    int listen();
    int accept();
    int distributeConnection(int ,int,bool);
    int runInServer();
    void start();

    int nListenSocket;

    TCPConnection *connection;
    std::map<uint32_t,TCPIOServer*> IOServer;
    Epoll *epollPtr;
    int fdNum;
    int IOServerNum;
};

#endif