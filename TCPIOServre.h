/*
 * @Descripttion: 
 * @version: 
 * @Author: sueRimn
 * @Date: 2021-02-26 13:36:35
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-26 14:10:37
 */
#ifndef _TCPIOSERVER_H_
#define _TCPIOSERVER_H_
#include<map>
class Epoll;
class TCPConnection;
class TCPIOServer{
    public:
    TCPIOServer():start(0),stop(false){}
    void Init();
    void startHadle();
    void runInIOServer();
    
    Epoll *epollPtr;
    std::map<uint32_t TCPConnection*> connectManager;
    int startID;
    bool stop;
    
};

#endif