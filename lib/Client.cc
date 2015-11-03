#include <Client.h>
#include <Logger.h>
#include <SocketLevel.h>

static Logging::Logger logger("Client");

namespace Network
{

void Client::Init(int32_t PollTimeout) throw (Exception)
{
    poll.Init(PollTimeout);
    locker.Create();
}

ClientConnectionId Client::AddConnection(ClientConnection *NewConnection, ClientSessionManager *Manager)
{
    LockGuard guard(locker);

    const Connection *conn = NewConnection->GetHandle();

    if(!NewConnection->IsNonblocking()){
        try{
            logger << Logging::DEBUG << "starting new session for connection " << conn->GetHandle();
            Manager->OnSesstionStart(conn);
            logger << Logging::DEBUG << "new session started for connection " << conn->GetHandle();
        }catch(const Exception &ex){
            NewConnection->Close();
            throw ex;
        }
    }

    connectionsCounter++;

    ConnectionData connData;
    connData.connection = NewConnection;
    connData.id = connectionsCounter;

    connectionIds.insert(std::make_pair(connectionsCounter, NewConnection));

    if(!NewConnection->IsNonblocking()){
        connections.insert(std::make_pair(connData, Manager));
        poll.Add(*conn, Poll::IN);
    }else{
        newConnections.insert(std::make_pair(connData, Manager));
        poll.Add(*conn, Poll::OUT);
    }

    return connectionsCounter;
}

void Client::Run() throw (Exception)
{
    while(true){

        if(!poll.Wait())
            continue;

        LockGuard guard(locker);

        ConnectionsMapping::iterator it;
        for(it = connections.begin(); it != connections.end(); ){
            const Connection *conn = it->first.connection->GetHandle();
            ClientSessionManager *manager = it->second;

            if(!poll.Check(*conn, Poll::IN)){
               ++it;
               continue;
            }

            Packet packet = conn->Read();
            if(packet.GetAction() == AFFECTOR_ERROR){
                conn->Write(packet);
                ++it;
                continue;
            }

            if(conn->IsClosed()){
                try{
                    logger << Logging::DEBUG << "ending session for connection " << conn->GetHandle();
                    manager->OnSessionEnd(conn);
                    logger << Logging::DEBUG << "session ended for connection " << conn->GetHandle();
                }catch(const Exception &ex){
                    logger << Logging::ERROR << ex.msg;

                    conn->Close();
                    poll.Remove(*conn);
                    delete conn;
                    connectionIds.erase(it->first.id);
                    connections.erase(it++);
                    if(!connections.size())
                        return;

                    throw ex;
                }

                poll.Remove(*conn);
                delete conn;
                connectionIds.erase(it->first.id);
                connections.erase(it++);
                if(!connections.size())
                    return;

                continue;
            }

            try{
                logger << Logging::DEBUG << "processing action " << packet.GetAction() << " for connection " << conn->GetHandle();
                manager->OnPacketRecieved(conn, packet);
                logger << Logging::DEBUG << "action " << packet.GetAction() << " processed for connection " << conn->GetHandle();
            }catch(const Exception &ex){
                conn->Close();

                logger << Logging::ERROR << ex.msg;

                conn->Close();
                poll.Remove(*conn);
                delete conn;
                connectionIds.erase(it->first.id);
                connections.erase(it++);
                if(!connections.size())
                    return;

                throw ex;
            }

            ++it;
        }

        for(it = newConnections.begin(); it != newConnections.end(); ){
            const Connection *conn = it->first.connection->GetHandle();
            ClientSessionManager *manager = it->second;

            if(!poll.Check(*conn, Poll::OUT)){
                ++it;
               continue;
            }

            bool newConnection = false;
            try{
                logger << Logging::DEBUG << "connection " << conn->GetHandle() << " establishing";
                manager->OnSesstionStart(conn);
                logger << Logging::DEBUG << "connection " << conn->GetHandle() << " established";

                connections.insert(*it);

                poll.Remove(*conn, Poll::OUT);
                poll.Add(*conn, Poll::IN);

                newConnection = true;
            }catch(const Exception &ex){
                logger << Logging::ERROR << ex.msg;

                conn->Write(CommonPackets::Error(ACTION_ERROR, ex.msg));
            }

            if(!newConnection){
                conn->Close();
                poll.Remove(*conn);
                delete conn;
            }

            newConnections.erase(it);
        }
    }
}

void Client::Stop(ClientConnectionId StopConnection) throw (Exception)
{
    LockGuard guard(locker);
    ConnectionsIdsMapping::iterator idIt = connectionIds.find(StopConnection);
    if(idIt == connectionIds.end())
        return;

    ConnectionData connData;
    connData.connection = idIt->second;
    connData.id = StopConnection;

    ConnectionsMapping::iterator it = connections.find(connData);
    if(it != connections.end())
        connData.connection->Close();
}

void Client::Stop() throw (Exception)
{
    LockGuard guard(locker);

    ConnectionsMapping::iterator it = connections.begin();

    while(it != connections.end())
        it->first.connection->Close();

}

}
