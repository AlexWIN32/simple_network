/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Exception.h>
#include <Socket.h>
#include <Packet.h>

namespace Network
{
       
class Connection
{
protected:
    SocketHandle socket;
    mutable bool isClosed;    
public:        
    bool operator < (const Connection &connection) const
    {
        return socket < connection.socket;
    }
    bool operator == (const Connection &connection) const
    {
        return socket == connection.socket;
    }
    virtual ~Connection(){}
    Connection() : socket(0), isClosed(false){}    
    virtual void Close() const = 0;
    virtual void Write(const Packet &DataPacket) const throw (Exception) = 0; 
    virtual Packet Read() const throw (Exception) = 0;
    bool IsClosed() const {return isClosed;}
    SocketHandle GetHandle() const {return socket;}
};

class ServerConnection
{
protected:
    Connection *handle;
public:
    virtual ~ServerConnection(){delete handle;}
    ServerConnection() : handle(NULL) {}
    const Connection &GetHandle() const {return *handle;}    
    virtual Connection *Accept() const throw (Exception) = 0;
    void Close(){handle->Close();}
};

class ClientConnection
{
protected:
    Connection *handle;
    bool isNonblocking;
    ClientConnection() : handle(NULL), isNonblocking(false) {}
public:
    bool IsNonblocking() const {return isNonblocking;}
    void SetNonblockingState( bool IsNonblocing) { isNonblocking = IsNonblocing;}
    virtual ~ClientConnection() {delete handle;}
    const Connection *GetHandle() const {return handle;}  
    void Close(){handle->Close();}
    void Send(const Packet &Data) const throw (Exception){handle->Write(Data);}
    Packet Recieve() const throw (Exception){return handle->Read();}
};

template<class TConnection>
class ConnectionHandler
{
private:
    TConnection &connection;
public:
    ConnectionHandler(TConnection &Connection) : connection(Connection){}
    ~ConnectionHandler()
    {
        connection.Close();
    }
    TConnection &Get(){return connection;}
    const TConnection &Get() const {return connection;}    
};

};
