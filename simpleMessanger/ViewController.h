/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

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