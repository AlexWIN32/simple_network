/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <Exception.h>
#include <Connection.h>
#include <PollFd.h>
#include <vector>

namespace Network
{

DECLARE_EXCEPTION(PollException);
class Poll
{
public:
    enum Event
    {
        IN,
        OUT
    };

    static const int32_t UntillEvent = -1;
    static const int32_t NoTimeout = 0;
    Poll() :timeout(NoTimeout){}
    Poll(int32_t Timeout) : timeout(Timeout){}
    void Init(int32_t Timeout){timeout = Timeout;}
    void Add(const Connection &WaitConnection, Event ConnectionEvent);  
    void Remove(const Connection &RemoveConnection);
    void Remove(const Connection &ChangeConnection, Event RemoveEvent);
    bool Wait() throw (Exception);
    bool Check(const Connection &WaitConnection, Event ConnectionEvent);
    void Clear() {data.clear();}
private:
    Poll(const Poll&);
    Poll &operator = (const Poll &rh);
    
    int32_t timeout;    
    std::vector<pollfd> data;
};
    
    
}