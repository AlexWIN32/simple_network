#pragma once
#include <Exception.h>
#include <stdint.h>

namespace Network
{

DECLARE_EXCEPTION(SocketException);
    
#ifdef WIN32
typedef SOCKET SocketHandle;
#else
typedef int32_t SocketHandle;
#endif

};