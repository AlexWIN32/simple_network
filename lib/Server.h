/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Connection.h>
#include <Poll.h>
#include <Session.h>
#include <Query.h>
#include <Exception.h>
#include <map>

namespace Network
{

class Server
{
private:
    struct ConnectionWrapper
    {
    private:
        Connection *conn;
    public:
        ConnectionWrapper(Connection *Conn) {conn = Conn;}
        operator Connection * () const  {return conn;}        
        bool operator < (const ConnectionWrapper &Wrapper) const
        {
            return conn->GetHandle() < Wrapper.conn->GetHandle();
        }        
    };
    Poll poll;
    ServerConnection *listenConnection;
    typedef std::vector<Connection*> ConnectionsStorage;
    typedef std::map<ConnectionWrapper, ServerSessionsManager*> ConnectionsMapping;
    typedef std::map<ServerSessionsManager*, ActionsStorage> SessionsManagersStorage;    
    typedef std::vector<QueryManager*> QueryManagersStorage;
    ConnectionsMapping connections;    
    SessionsManagersStorage managers;    
    QueryManagersStorage queryManagers;
    ConnectionsStorage newConnections;
    ServerSessionsManager* FindManager(ActionId Action);
public:
    Server():listenConnection(NULL){}
    ~Server(){Stop();}
    void Init(int32_t PollTimeout, ServerConnection *ListeningConnection);
    void AddManager(ServerSessionsManager *NewManager)
    {
        managers.insert(std::make_pair(NewManager, NewManager->GetNewSessionEvents())); 
    }
    void AddManager(QueryManager *NewManager){queryManagers.push_back(NewManager);}
    void Run() throw (Exception);
    void Stop() throw (Exception);
};

};
