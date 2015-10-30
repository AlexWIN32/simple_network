#pragma once

#include <Exception.h>
#include <Packet.h>
#include <Connection.h>
#include <map>

namespace Network
{

class SessionsAffector
{
public:
    virtual ~SessionsAffector(){}
    virtual void OnPacketRecieved(const Connection *ClientConnection, const Packet &Data) throw (Exception) = 0;
    virtual ActionsStorage GetAffectEvents() const = 0;
};

class PacketProcessor
{
private:
    typedef std::map<SessionsAffector*, ActionsStorage> AffectorsStorage;
    AffectorsStorage affectors;
public:
    virtual ~PacketProcessor(){}
    void AddAffector(SessionsAffector *Affector){affectors.insert(std::make_pair(Affector, Affector->GetAffectEvents()));}
    void RemoveAffector(SessionsAffector *Affector)
    {
        AffectorsStorage::iterator it = affectors.find(Affector);
        if(it != affectors.end())
            affectors.erase(it);
    }
    void RemoveAllAffectors() {affectors.clear();}
    void ProcessNewPacket(const Connection *ClientConnection, const Packet &Data) throw (Exception)
    {
        AffectorsStorage::const_iterator ci;
        for(ci = affectors.begin(); ci != affectors.end(); ++ci){
            SessionsAffector *affector = ci->first;
            const ActionsStorage &actions = ci->second;
            if(std::find(actions.begin(), actions.end(), Data.GetAction()) != actions.end()){
                affector->OnPacketRecieved(ClientConnection, Data);
                return;
            }
        }

        OnPacketRecieved(ClientConnection, Data);
    }
    virtual void OnPacketRecieved(const Connection *ClientConnection, const Packet &Data) throw (Exception) = 0;
};

class ServerSessionsManager : public PacketProcessor
{
public:
    virtual ~ServerSessionsManager(){}
    virtual ActionsStorage GetNewSessionEvents() const = 0;
    virtual void SendToAllSessions(const Packet &Data) throw (Exception) = 0;
    virtual void OnSesstionStart(const Connection *ClientConnection, const Packet &Data) throw (Exception) = 0;
    virtual void OnSessionEnd(const Connection *ClientConnection) throw (Exception) = 0;
};

class ClientSessionManager : public PacketProcessor
{
public:
    virtual ~ClientSessionManager(){}
    virtual void OnSesstionStart(const Connection *ClientConnection) throw (Exception) = 0;
    virtual void OnSessionEnd(const Connection *ClientConnection) throw (Exception) = 0;
};

}
