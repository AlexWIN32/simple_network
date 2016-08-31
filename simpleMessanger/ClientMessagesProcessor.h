/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Session.h>

DECLARE_EXCEPTION(NameIsUsedException)
DECLARE_EXCEPTION(ClientMessagesProcessorException)

class ViewController;
class ClientMessagesProcessor : public Network::ClientSessionManager
{
private:
    std::string login;
    ViewController* controller;
    const Network::Connection *connection;
    virtual Network::ActionsStorage GetNewSessionEvents() const;    
    virtual void OnSesstionStart(const Network::Connection *ClientConnection) throw (Exception);
    virtual void OnSessionEnd(const Network::Connection *ClientConnection) throw (Exception);
    virtual void OnPacketRecieved(const Network::Connection *ClientConnection, const Network::Packet &Data) throw (Exception);
public:
    void Init(const std::string &Login, ViewController* Controller)
    {
        login = Login; 
        controller = Controller;        
    }
    void SendMessage(const std::string &Message) throw (Exception);
    virtual ~ClientMessagesProcessor(){}    
};
