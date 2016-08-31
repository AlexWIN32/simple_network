/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Server.h>
#include <Packet.h>
#include <Logger.h>
#include <Serializing.h>

static Logging::Logger logger("Server");

namespace Network
{

void Server::Init(int32_t PollTimeout, ServerConnection *ListeningConnection)
{
    poll.Init(PollTimeout);
    listenConnection = ListeningConnection;
    poll.Add(listenConnection->GetHandle(), Poll::IN);
}

ServerSessionsManager* Server::FindManager( ActionId Action)
{
    SessionsManagersStorage::const_iterator ci;
    for(ci = managers.begin(); ci != managers.end(); ++ci){
        ServerSessionsManager *manager = ci->first;
        const ActionsStorage &acts = ci->second;

        if(std::find(acts.begin(), acts.end(), Action) != acts.end())
            return manager;
    }

    return NULL;
}

void Server::Run() throw (Exception)
{    
    while(true){
        if(!poll.Wait())
            continue;

        if(poll.Check(listenConnection->GetHandle(), Poll::IN)){

            Connection *newConnection;
            try{
                newConnection = listenConnection->Accept();
            }catch(const Exception &ex){
                logger << Logging::ERROR << ex.msg;
                continue;
            }

            poll.Add(*newConnection, Poll::IN);
            newConnections.push_back(newConnection);

            logger << Logging::DEBUG << "new connection";
            continue;
        }

        ConnectionsMapping::iterator conn_it;
        for(conn_it = connections.begin(); conn_it != connections.end(); ){
            Connection *conn = conn_it->first;
            ServerSessionsManager *manager = conn_it->second;

            if(!poll.Check(*conn, Poll::IN)){
                ++conn_it;
                continue;
            }

            Packet packet = conn->Read();
            if(packet.GetAction() == AFFECTOR_ERROR){
                conn->Write(packet);
                ++conn_it;
                continue;
            }

            if(conn->IsClosed()){
                try{

                    logger << Logging::DEBUG << "end session for " << conn->GetHandle();
                    manager->OnSessionEnd(conn);
                    logger << Logging::DEBUG << "end session for " << conn->GetHandle() << " success";

                }catch(const Exception &ex){
                    logger << Logging::ERROR << ex.msg;
                }

                poll.Remove(*conn);
                delete conn;
                connections.erase(conn_it++);
                continue;
            }

            try{

                logger << Logging::DEBUG << "processing action " << packet.GetAction();
                manager->ProcessNewPacket(conn, packet);
                logger << Logging::DEBUG << "processing action " << packet.GetAction() << " success";

            }catch(const Exception &ex){
                logger << Logging::ERROR << ex.msg;

                conn->Write(CommonPackets::Error(ACTION_ERROR, "action " +
                           NumericParser<ActionId>::ToString(packet.GetAction()) +":" + ex.msg));

                conn->Close();
                poll.Remove(*conn);
                delete conn;
                connections.erase(conn_it++);
                continue;
            }
            ++conn_it;
        }

        ConnectionsStorage::iterator it;
        for(it = newConnections.begin(); it != newConnections.end(); ){
            Connection* conn = *it;
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

            bool newConnection = false;
            QueryManagersStorage::const_iterator ci;
            try{

                for(ci = queryManagers.begin(); ci != queryManagers.end(); ++ci)
                    if((*ci)->Process(packet, conn)){
                        logger << Logging::DEBUG << "query " << packet.GetAction() << " processed";
                        break;
                    }
            }catch(const Exception &ex){

                std::ostringstream errMsg;
                errMsg << "query " << packet.GetAction() << ": " << ex.msg;
                logger << Logging::ERROR << errMsg.str();

                conn->Write(CommonPackets::Error(QUERY_ERROR, errMsg.str()));
            }

            if(ci == queryManagers.end()){
                ServerSessionsManager* manager = FindManager(packet.GetAction());
                if(!manager){
                    std::ostringstream errMsg;
                    errMsg <<  "manager not found for " << packet.GetAction();
                    logger << Logging::ERROR << errMsg.str();

                    conn->Write(CommonPackets::Error(ACTION_NOT_FOUND_ERROR, errMsg.str()));
                }else{
                    try{
                        logger << Logging::DEBUG << "connection " << conn->GetHandle() << " establishing with action " << packet.GetAction();
                        manager->OnSesstionStart(conn, packet);
                        logger << Logging::DEBUG << "connection " << conn->GetHandle() << " established";

                        connections.insert(std::make_pair(conn, manager));
                        newConnection = true;
                    }catch(const Exception &ex){
                        logger << Logging::ERROR << ex.msg;

                        conn->Write(CommonPackets::Error(ACTION_ERROR, "action " +
                            NumericParser<ActionId>::ToString(packet.GetAction()) +":" + ex.msg));
                    }
                }
            }

            if(!newConnection){
                conn->Close();
                poll.Remove(*conn);
                delete conn;
            }

            it = newConnections.erase(it);
        }
    }
}

void Server::Stop() throw (Exception)
{
    ConnectionsStorage::const_iterator ci;
    for(ci = newConnections.begin(); ci != newConnections.end(); ++ci){
        Network::Connection *conn = *ci;
        conn->Close();
        delete conn;
    }

    ConnectionsMapping::const_iterator ci2;
    for(ci2 = connections.begin(); ci2 != connections.end(); ++ci2){
        Network::Connection *conn = ci2->first;
        conn->Close();
        delete conn;
    }
}

}

