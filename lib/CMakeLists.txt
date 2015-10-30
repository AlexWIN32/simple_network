
add_library(serializing STATIC Serializing.cc)

if(WIN32)
    add_library(locker STATIC win32/win32_locker.cc)
else()
    add_library(locker STATIC unix/unix_locker.cc)
endif()

add_library(logging STATIC Logger.cc)

if(WIN32)
    set(SOCKET_CC "win32/win32_socket.cc")
else()
    set(SOCKET_CC "unix/unix_socket.cc")
endif()

add_library(simple_network STATIC ${SOCKET_CC} SimpleTcpConnection.cc
            SSLConnection.cc Poll.cc Server.cc Client.cc Packet.cc)
target_link_libraries(simple_network logging locker serializing ${SSL_LIB})
