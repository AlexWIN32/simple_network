#pragma once
#include <Exception.h>
#include <Socket.h>

struct pollfd;

namespace SocketLevel
{

DECLARE_EXCEPTION(SocketException);
    
void close_socket(Network::SocketHandle socket);

int32_t write_data(Network::SocketHandle socket, const void * buff, size_t buff_size) throw (Exception);

int32_t read_data(Network::SocketHandle socket, void * buff, size_t buff_size) throw (Exception);

Network::SocketHandle duplicate_socket(Network::SocketHandle sd) throw (Exception);

Network::SocketHandle create_tcp_server_socket(int16_t port, int32_t qeue_len) throw (Exception);

Network::SocketHandle accept_from_tcp_server_socket(Network::SocketHandle socket) throw (Exception);

Network::SocketHandle create_tcp_client_socket(const std::string &hostname, int16_t port) throw (Exception);

int32_t poll_socket(struct pollfd * fds, int32_t nfds, int32_t timeout) throw (Exception);


};