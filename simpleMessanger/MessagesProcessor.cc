/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <MessagesProcessor.h>
#include <Serializing.h>
#include <Actions.h>
#include <Variables.h>
#include <Logger.h>

static Logging::Logger logger("ServerProcessor");

Network::ActionsStorage MessagesProcessor::GetNewSessionEvents() const
{
    Network::ActionsStorage actions;
    actions.push_back(ACT_USER_LOGIN);
    actions.push_back(ACT_USER_LOGOUT);
    actions.push_back(ACT_NEW_MESSAGE);
    return actions;
}

void MessagesProcessor::SendToAllSessions(const Network::Packet &Data) throw (Exception)
{
    UsersDataStorage::const_iterator ci;
    for(ci = users.begin(); ci != users.end(); ++ci)
        ci->connection->Write(Data);
}

void MessagesProcessor::OnSesstionStart(const Network::Connection *ClientConnection, const Network::Packet &Data) throw (Exception)
{
    if(Data.GetAction() != ACT_USER_LOGIN)
        throw MessagesProcessorError("Invalid message");

    std::string name = static_cast<std::string>(Data[VAR_USER_NAME]);

    UsersDataStorage::const_iterator ci;
    for(ci = users.begin(); ci != users.end(); ++ci)
        if(ci->name == name){
            ClientConnection->Write(Network::Packet(ERR_NAME_IS_USED));
            return;
        }

    SendToAllSessions(Data);

    UserData newUser;
    newUser.name = name;
    newUser.connection = ClientConnection;
    users.push_back(newUser);

    logger << Logging::DEBUG << "sending ACTION_SUCCESS";
    ClientConnection->Write(Network::Packet(Network::ACTION_SUCCESS));
}

void MessagesProcessor::OnSessionEnd(const Network::Connection *ClientConnection) throw (Exception)
{
    std::string logoutUserName;
    UsersDataStorage::iterator it;
    for(it = users.begin(); it != users.end();it++)
        if(*it->connection == *ClientConnection){
            logoutUserName = it->name;
            users.erase(it);
            break;
        }

    Network::Packet logoutPacket(ACT_USER_LOGOUT);
    logoutPacket.CreateVariable(VAR_USER_NAME, logoutUserName);
    SendToAllSessions(logoutPacket);
}

void MessagesProcessor::OnPacketRecieved(const Network::Connection *ClientConnection, const Network::Packet &Data) throw (Exception)
{
    if(Data.GetAction() != ACT_NEW_MESSAGE)
        throw MessagesProcessorError("Invalid message");

    SendToAllSessions(Data);
}

bool MessagesProcessor::NameCheckQueryProcessor::Process(const Network::Packet& Data, const Network::Connection *ClientConnection) throw (Exception)
{
    if(Data.GetAction() != QRY_NAME_CHECK)
        return false;

    std::string name = static_cast<std::string>(Data[VAR_USER_NAME]);

    MessagesProcessor::UsersDataStorage::const_iterator ci;
    for(ci = processor->users.begin(); ci != processor->users.end(); ++ci)
        if(ci->name == name){
            ClientConnection->Write(Network::Packet(QRY_ASK_NAME_IS_USED));
            return true;
        }

    ClientConnection->Write(Network::Packet(QRY_ASK_NAME_IS_FREE));
    return true;
}
