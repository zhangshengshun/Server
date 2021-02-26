/*
 * @Descripttion: 
 * @version: 
 * @Author: sueRimn
 * @Date: 2021-02-26 13:32:32
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-26 14:09:50
 */
#include"TCPConnection.h"
#include"Error.h"
#include"Epoll.h"
#include"TCPIOServer.h"

TCPConnection::TCPConnection(Epoll *epollInServer):event(epollInServer){
    m_getNewPackage=true;
    m_getReadHeader=false;
    
    m_nHeadSize=0;
    m_nReadOffset=0;
    m_nContentLength=0;
    m_sendIovList.clear();
}

TCPConnection::TCPConnection(TCPIOServer* IOServer,Epoll* epollPtr):event(epollPtr){
    m_getNewPackage=true;
    m_getReadHeader=false;
    
    m_nHeadSize=0;
    m_nReadOffset=0;
    m_nContentLength=0;
    m_sendIovList.clear();

    server=IOServer;
}

int TCPconnection::readData(){
    
    if(this->readTCP()<0){
        return FAILED;
    }
    return SUCCESSFUL;
}

int TCPconnection::sendData(){
    int iovcnt = m_sendIovList.size();
    if(iovcnt>IOV_MAX){
        iovcnt=IOV_MAX;
    }
    struct iovec* pIovec=new struct iovec[iovcnt];
    memset(pIovec,0,sizeof( struct iovec ) * iovcnt);
   
    int i=0;
    for(auto begin=m_sendIovList.begin();begin!=m_sendIovList.end();begin++,i++){
        if(i<iovcnt){
            pIovec[i]=begin->m_Iovec;
        }
        else{
            break;
        }
    }

    
    int rt=::writev(sockfd,pIovec, iovcnt);
    
    delete[] pIovec;
    pIovec=nullptr;
    
    if(rt<0){
        //this->event.closeWevent();
        return FAILED;
    }

    //标记已经发送的消息
    uint32_t writen=(uint32_t) rt;
    for ( auto it = m_sendIovList.begin();it != m_sendIovList.end(); ++it ){
        if ( writen > it->m_Iovec.iov_len )
        {
            writen -= it->m_Iovec.iov_len;
            it->m_Iovec.iov_base = nullptr;
            it->m_Iovec.iov_len = 0;
        }
        else
        {
            it->m_Iovec.iov_base = static_cast<char*>( it->m_Iovec.iov_base ) + writen;
            it->m_Iovec.iov_len -= static_cast<size_t>( writen );
            break;
        }
    }

    //将已发送的消息删除
    for( auto it = m_sendIovList.begin();it != m_sendIovList.end();){
        if(it->m_Iovec.iov_len==0){
            delete [] it->m_pCompleteBuffer;
            it->m_pCompleteBuffer=nullptr;
            m_sendIovList.erase(it++);
        }
        else{
            break;
        }
    }
    return rt;
}

int TCPconnection::readTCP(){
    if(m_getNewPackage){
        m_getNewPackage=false;
        m_getReadHeader=true;

        m_nReadOffset=0;
        m_nContentLength=0;

        m_nHeadSize=HEADER_SIZE;

        m_InReq.ioBuf=nullptr;
        memset(&m_InReq.m_msgHeader,0,m_nHeadSize); 
    }
    
    int rt;
    //cout<<"cli==null"<<endl;
    if(m_getReadHeader){
        //cout<<"cli==null"<<endl;
        rt=this->readTCPHead();
        if(rt<=0){
            return rt;
        }
    }

    if(!m_getReadHeader&&!m_getNewPackage){
        rt=this->readTCPContent();
        if(rt<=0){
            return rt;
        }
    }

    return SUCCESSFUL;
}

int TCPconnection::readTCPHead(){
    
    int rt=this->read((char*)( &(m_InReq.m_msgHeader))+m_nReadOffset,m_nHeadSize-m_nReadOffset);
    
    if(rt<=0){
        return FAILED;
    }
    m_nReadOffset+=(uint32_t)rt;
    if(m_nReadOffset==m_nHeadSize){
        m_nReadOffset=0;
        m_getReadHeader=false;
        m_nContentLength=m_InReq.m_msgHeader.legnth;
        
        if(m_InReq.m_msgHeader.cmd==1&& m_InReq.m_msgHeader.legnth==0){
            readBack();
            delete[] m_InReq.ioBuf;
            m_getNewPackage = true;
        }
        if(0 == m_nContentLength){
            m_getNewPackage = true;
        }
        else if(m_nContentLength > 0){
            m_InReq.ioBuf=new char[m_nContentLength];
            memset(m_InReq.ioBuf,0,m_nContentLength);
        }
    }
    return SUCCESSFUL;
}

int TCPconnection::readTCPContent(){
    int rt=this->read(m_InReq.ioBuf+m_nReadOffset,m_nContentLength-m_nReadOffset);
    if(rt<=0){
        return FAILED;
    }
    m_nReadOffset += ( uint32_t )rt;
    if ( m_nContentLength == m_nReadOffset ){
        m_nReadOffset = 0;
        m_getNewPackage = true;
        //cout<<m_InReq.m_msgHeader.recvfrom<<"      "<<m_InReq.m_msgHeader.sendform<<"    "<<m_InReq.ioBuf<<endl;
        //读完内容后执行回调
        this->readBack();
        delete [] m_InReq.ioBuf;
        return FAILED;
    }
    return SUCCESSFUL;
}

int TCPconnection::readBack(){
    TCPconnection* connect=nullptr;
    //cout<<server->fdMap.size()<<endl;
    if(this->m_InReq.m_msgHeader.sendform==this->server->fdMap.size()){
        
        connect=server->fdMap[1];
    }
    else{
        connect=server->fdMap[m_InReq.m_msgHeader.sendform];
    }
    
    InReq reqSerevr;
    reqSerevr.m_msgHeader.cmd=2;
    reqSerevr.m_msgHeader.legnth=m_InReq.m_msgHeader.legnth;
    reqSerevr.m_msgHeader.recvfrom=m_InReq.m_msgHeader.recvfrom;
    reqSerevr.m_msgHeader.sendform=m_InReq.m_msgHeader.sendform;

    int msgLength=HEADER_SIZE+reqSerevr.m_msgHeader.legnth;
    
    char *sendBuf = new char[msgLength];
    memset(sendBuf,0,msgLength);
    memcpy(sendBuf, &reqSerevr.m_msgHeader, HEADER_SIZE);
    
    if(this->m_InReq.m_msgHeader.legnth!=0&&this->m_InReq.ioBuf!=nullptr){
        memcpy(sendBuf+sizeof(MsgHeader), m_InReq.ioBuf, m_InReq.m_msgHeader.legnth);
    }

    connect->m_sendIovList.push_back(Iov(sendBuf,msgLength));
    if(connect->event.openWevent()<0){
        return FAILED;
    }
    return SUCCESSFUL;
}

int TCPconnection::read(char* buf, size_t len){
    int readNum=::read(sockfd,buf,len);
    return readNum;
}

int TCPconnection::write(const char* buf, size_t len){
    int writeNum=::write(sockfd,buf,len);
    return writeNum;
}

int TCPconnection::close(void){
    if(sockfd==-1){
        return SUCCESSFUL;
    }
    if(::close(sockfd)){
        std::cerr<< "TCPSocket::close::close"<<std::endl;
        return FAILED;
    }
    sockfd = -1;
    return SUCCESSFUL;
}

int TCPconnection::setNonblock(void){
    int val;
    if((val=fcntl(sockfd,F_GETFL,0))<0){
        cout<<"TCPSocket::setNonBlock::fcntl-F_GETFL"<<endl;
        return val;
    }

    val|=O_NONBLOCK;
    if(fcntl( sockfd, F_SETFL, val ) < 0){
        return FAILED;
    }
    return SUCCESSFUL;
}

int TCPconnection::disableLinger(){
    int val=1;
    struct linger ling = {0, 0};
    if(setsockopt( sockfd,SOL_SOCKET, SO_LINGER, &ling, sizeof( ling ) ) < 0 ){
        return FAILED;
    }
    return SUCCESSFUL;
}

int TCPconnection::disableNagle(){
    int val = 1;
    if ( setsockopt( 
        sockfd, 
        IPPROTO_TCP, 
        TCP_NODELAY, 
        ( const void* )&val, sizeof( val ) ) < 0 )
    {
        //ERROR_LOG( "TCPSocket::disableNagle::setsockopt" );
        return FAILED;
    }

    return SUCCESSFUL;
}

int TCPconnection::enableReuseaddr(){
    int val = 1;
    if ( setsockopt( 
        sockfd, 
        SOL_SOCKET, 
        SO_REUSEADDR, 
        ( const void* )&val, sizeof( val ) ) < 0 )
    {
        //ERROR_LOG( "TCPSocket::enableReuseaddr::setsockopt" );
        return FAILED;
    }

    return SUCCESSFUL;
}

