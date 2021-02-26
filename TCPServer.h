/*
 * @Descripttion: 
 * @version: 
 * @Author: sueRimn
 * @Date: 2021-02-26 13:39:18
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-26 14:02:04
 */
#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

const int maxIOServerNum=3;

class TCPIOServer;
class TCPServer{
    public:
    Server();
    
    void Init();
    int listen();
    int accept();
    int distributeConnection();

    int nLIstenSOcket;
    std::map<uint32_t,TCPIOServer*> IOServer;
    int fdNum;
    int IOServerNum;
};

#endif