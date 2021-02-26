/*
 * @Descripttion: 
 * @version: 
 * @Author: sueRimn
 * @Date: 2021-02-26 13:32:21
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-26 13:36:21
 */
#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_
#include<iostream>
#include <sys/epoll.h>
#include<map>
#include<list>
#include<netinet/in.h>
#include<sys/uio.h>
#include<cstring>

const unsigned int HEADER_SIZE=4*sizeof(uint32_t);

struct MsgHeader{
    uint32_t cmd;//消息类型
    uint32_t legnth;//消息长度
    uint32_t recvfrom;//要发送给谁的ID
    uint32_t sendform;//自己的ID
};

typedef struct Iov{
    Iov(){
        m_pCompleteBuffer=nullptr;
        memset(&m_Iovec,0,sizeof(m_Iovec));
    }

    Iov(char* buf,size_t len){
        m_pCompleteBuffer=buf;
        m_Iovec.iov_base=buf;
        m_Iovec.iov_len=len;
    }

    ~Iov(){
        m_Iovec.iov_base=nullptr;
        m_Iovec.iov_len=0;
        m_pCompleteBuffer=nullptr;
    }

    struct iovec m_Iovec;
    char* m_pCompleteBuffer;
}Iov;

struct InReq
{
    MsgHeader m_msgHeader;
    char* ioBuf;
};

class TCPConnection{
    connectfd();
};

#endif