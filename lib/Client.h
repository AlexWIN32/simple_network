/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Connection.h>
#include <Session.h>
#include <map>
#include <Poll.h>
#include <Locker.h>

namespace Network
{
typedef int32_t ClientConnectionId;

DECLARE_EXCEPTION(ConnectionNotFoundException);

class Client
{
private:
    struct ConnectionData
    {
        ClientConnectionId id;
        ClientConnection* connection;
        ConnectionData():id(0){}
        bool operator < (const ConnectionData &Data) const
        {
            return id < Data.id && connection < Data.connection;
        }
    };
    Poll poll;
    typedef std::map<ConnectionData, ClientSessionManager*> ConnectionsMapping;
    ConnectionsMapping connections, newConnections;
    typedef std::map<ClientConnectionId, ClientConnection*> ConnectionsIdsMapping;
    ConnectionsIdsMapping connectionIds;
    ClientConnectionId connectionsCounter;
    Locker locker;
public:
    Client():connectionsCounter(0){}
    ~Client(){Stop();}
    void Init(int32_t PollTimeout) throw (Exception);
    ClientConnectionId AddConnection(ClientConnection *NewConnection, ClientSessionManager *Manager);
    void Run() throw (Exception);
    void Stop(ClientConnectionId StopConnection) throw (Exception);
    void Stop() throw (Exception);
};

}