/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Connection.h>

namespace Network
{

class QueryManager
{
public:
    virtual ~QueryManager(){}
    virtual bool Process(const Packet& Data, const Connection *ClientConnection) throw (Exception)= 0;
};
    
};
