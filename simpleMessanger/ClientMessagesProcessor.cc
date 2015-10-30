#include <Actions.h>
#include <Variables.h>
#include <ClientMessagesProcessor.h>
#include <ViewController.h>
#include <Logger.h>

static Logging::Logger logger("ClientProcessor");

Network::ActionsStorage ClientMessagesProcessor::GetNewSessionEvents() const
{
    return Network::ActionsStorage();
}

void ClientMessagesProcessor::OnSesstionStart(const Network::Connection *ClientConnection) throw (Exception)
{
    logger << Logging::DEBUG << "sending USER_LOGIN";

    Network::Packet loginPacket(ACT_USER_LOGIN);
    loginPacket.CreateVariable(VAR_USER_NAME, login);

    ClientConnection->Write(loginPacket);

    connection = ClientConnection;
}

void ClientMessagesProcessor::OnSessionEnd(const Network::Connection *ClientConnection) throw (Exception)
{
    controller->ShowLostConnection();
}

void ClientMessagesProcessor::OnPacketRecieved(const Network::Connection *ClientConnection, const Network::Packet &Data) throw (Exception)
{

    if(Data.GetAction() == ERR_NAME_IS_USED)
        throw NameIsUsedException("");

    if(Data.GetAction() == ACT_NEW_MESSAGE){

        std::string login = static_cast<std::string>(Data[VAR_USER_NAME]);
        std::string message = static_cast<std::string>(Data[VAR_MESSAGE]);

        controller->ShowMessage(login, message);
    }else if(Data.GetAction() == ACT_USER_LOGIN){

        std::string login = static_cast<std::string>(Data[VAR_USER_NAME]);

        controller->ShowUserLogin(login);
    }else if(Data.GetAction() == ACT_USER_LOGOUT){

        std::string login = static_cast<std::string>(Data[VAR_USER_NAME]);

        controller->ShowUserLogout(login);
    }
    else if(Data.GetAction() != Network::ACTION_SUCCESS)
        throw ClientMessagesProcessorException("Invalid action");

}

void ClientMessagesProcessor::SendMessage(const std::string &Message) throw (Exception)
{
    Network::Packet messagePacket(ACT_NEW_MESSAGE);

    messagePacket.CreateVariable(VAR_USER_NAME, login);
    messagePacket.CreateVariable(VAR_MESSAGE, Message);

    connection->Write(messagePacket);
}
