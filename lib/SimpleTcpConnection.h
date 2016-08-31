/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Connection.h>

namespace Network
{
    
class SimpleTcpConnection : public Connection
{
public:
    virtual ~SimpleTcpConnection(){}
    virtual void Close() const;
    virtual void Write(const Packet &DataPacket) const throw (Exception); 
    virtual Packet Read() const throw (Exception);
    void Init(SocketHandle Socket);
};

class SimpleTcpServerConnection : public ServerConnection
{
public: 
    virtual ~SimpleTcpServerConnection(){}
    void Init(int16_t Port, int32_t QueueLen) throw (Exception);
    virtual Connection *Accept() const throw (Exception);    
};

class SimpleTcpClientConnection : public ClientConnection
{    
public: 
    SimpleTcpClientConnection(){}
    virtual ~SimpleTcpClientConnection(){}
    virtual void Connect(const std::string &Host, int16_t Port) throw (Exception);    
};

}
