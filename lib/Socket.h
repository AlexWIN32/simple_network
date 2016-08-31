/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

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