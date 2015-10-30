#pragma once
#include <Session.h>
#include <Query.h>
#include <Connection.h>
#include <vector>
#include <NetworkAction.h>

DECLARE_EXCEPTION(MessagesProcessorError);

class MessagesProcessor : public Network::ServerSessionsManager
{
private:
    struct UserData
    {
        std::string name;
        const Network::Connection *connection;
    };
    typedef std::vector<UserData> UsersDataStorage;
    UsersDataStorage users;    
public:    
    class NameCheckQueryProcessor : public Network::QueryManager
    {
    private:
        MessagesProcessor *processor;
    public:
        NameCheckQueryProcessor(MessagesProcessor *Processor){processor = Processor;}
        virtual ~NameCheckQueryProcessor(){}
        virtual bool Process(const Network::Packet& Data, const Network::Connection *ClientConnection) throw (Exception);
    };
    virtual ~MessagesProcessor(){}
    virtual Network::ActionsStorage GetNewSessionEvents() const;
    virtual void SendToAllSessions(const Network::Packet &Data) throw (Exception);
    virtual void OnSesstionStart(const Network::Connection *ClientConnection, const Network::Packet &Data) throw (Exception);
    virtual void OnSessionEnd(const Network::Connection *ClientConnection) throw (Exception);
    virtual void OnPacketRecieved(const Network::Connection *ClientConnection, const Network::Packet &Data) throw (Exception);
};
