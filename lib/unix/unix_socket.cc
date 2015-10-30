#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <SocketLevel.h>
#include <PollFd.h>

namespace SocketLevel
{

void close_socket(Network::SocketHandle socket)
{
    //close(socket);
    shutdown(socket, SHUT_RDWR);
}

int32_t write_data(Network::SocketHandle socket, const void * buff, size_t buff_size) throw (Exception)
{
    int32_t res = send(socket, buff, buff_size, 0);

    if(res < 0)
        throw SocketException(std::string("write failed: ") + strerror(errno));
    
    return res;
}

int32_t read_data(Network::SocketHandle socket, void * buff, size_t buff_size) throw (Exception)
{
    int32_t res = recv(socket, buff, buff_size, 0);

    if(res < 0)
        throw SocketException(std::string("read failed: ") + strerror(errno));

    return res;
}

Network::SocketHandle duplicate_socket(Network::SocketHandle sd) throw (Exception)
{
    return dup(sd);
}

int32_t poll_socket(struct pollfd * fds, int32_t nfds, int32_t timeout) throw (Exception)
{
    int32_t res = poll(fds, nfds, timeout);
    if(res < 0){
        if (res == EINTR)
            return 0;

        throw SocketException(std::string("poll failed: ") + strerror(errno));
    }    
    
    return res;
}

Network::SocketHandle create_tcp_server_socket(int16_t port, int32_t qeue_len) throw (Exception)
{
    Network::SocketHandle sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0)
        throw SocketException(std::string("cant create socket: ") + strerror(errno));
    
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);        
    
    int on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<void *>(&on), sizeof(int));
    
    if (bind(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0)
        throw SocketException(std::string("cant bind socket: ") + strerror(errno));

    if(listen(sock, qeue_len) < 0)
        throw SocketException(std::string("cant listen socket: ") + strerror(errno));
    return sock;
}

Network::SocketHandle accept_from_tcp_server_socket(Network::SocketHandle socket) throw (Exception)
{
    int new_sd = accept(socket, NULL, NULL);    
    if(new_sd < 0)
        throw SocketException(std::string("cant accept: ") + strerror(errno));
           
    return new_sd;
}

Network::SocketHandle create_tcp_client_socket(const std::string &hostname, int16_t port) throw (Exception)
{
    addrinfo hints;        
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
  
    addrinfo *res;
    if(getaddrinfo(hostname.c_str(), NULL, &hints, &res) != 0)
        throw SocketException("host not found");
      
    Network::SocketHandle sock = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if(sock < 0)
        throw SocketException(std::string("cant create socket: ") + strerror(errno));
    
    struct sockaddr_in servaddr;   
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr = (reinterpret_cast<sockaddr_in*>(res->ai_addr))->sin_addr;
    servaddr.sin_port = htons(port);            
    
    if(connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        throw SocketException(std::string("cant conect: ") + strerror(errno));
  
    return sock;
}

};
