#include <SimpleTcpConnection.h>
#include <PacketProcessor.h>
#include <Serializing.h>
#include <SocketLevel.h>
#include <Logger.h>
#include <vector>

static Logging::Logger logger("connection");

namespace Network
{    

void SimpleTcpConnection::Init(SocketHandle Socket)
{
    socket = Socket;
    isClosed = false;
}

void SimpleTcpConnection::Close() const
{
    SocketLevel::close_socket(socket);
    isClosed = true;
}

void SimpleTcpConnection::Write(const Packet &DataPacket) const throw (Exception)
{
    if(isClosed)
        return;
    
    PacketProcessor::Write<SocketHandle>(socket, DataPacket, SocketLevel::write_data);       
}

Packet SimpleTcpConnection::Read() const throw (Exception)
{
    if(isClosed)
        return Packet();
    
    return PacketProcessor::Read(isClosed, socket, SocketLevel::read_data); 
}

void SimpleTcpServerConnection::Init(int16_t Port, int32_t QueueLen) throw (Exception)
{
    SimpleTcpConnection *tcpHandle = new SimpleTcpConnection();
    tcpHandle->Init(SocketLevel::create_tcp_server_socket(Port, QueueLen));
    handle = tcpHandle;
}

Connection* SimpleTcpServerConnection::Accept() const throw (Exception)
{
    SimpleTcpConnection *newConnection = new SimpleTcpConnection();
    newConnection->Init(SocketLevel::accept_from_tcp_server_socket(handle->GetHandle()));
    return newConnection;
}

void SimpleTcpClientConnection::Connect(const std::string &Host, int16_t Port) throw (Exception)
{
    SimpleTcpConnection *tcpConnection = new SimpleTcpConnection();  
    tcpConnection->Init(SocketLevel::create_tcp_client_socket(Host, Port, isNonblocking));
    handle = tcpConnection;
}

};