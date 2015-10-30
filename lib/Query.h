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
