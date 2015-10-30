#pragma once
#include <Exception.h>
#include <string>

class ClientMessagesProcessor;
class ViewController
{
protected:
    ClientMessagesProcessor *msgProcessor;
public:
    ViewController(){}
    virtual ~ViewController(){}
    virtual void ShowMessage(const std::string &Login, const std::string &Message) throw (Exception) = 0;
    virtual void ShowUserLogin(const std::string &Login) throw (Exception) = 0;
    virtual void ShowUserLogout(const std::string &Login) throw (Exception) = 0;
    virtual void ShowLostConnection() throw (Exception) = 0;
};