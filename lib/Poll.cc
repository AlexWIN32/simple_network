/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Poll.h>
#include <SocketLevel.h>
#include <PollFd.h>
#include <algorithm>
#include <iostream>

namespace Network
{

static int16_t get_event( Poll::Event event ) throw(Exception)
{
    short event_mask;

    switch(event)
    {
    case Poll::IN:
        event_mask = POLLIN;
        break;
    case Poll::OUT:
        event_mask = POLLOUT;
        break;
    }

    return event_mask;
}
    
void Poll::Add(const Connection &WaitConnection, Event ConnectionEvent)
{
    std::vector<pollfd>::iterator it;
    for(it = data.begin(); it != data.end(); ++it)
        if(it->fd == WaitConnection.GetHandle()){
            it->events |= get_event(ConnectionEvent);
            return;
        }

    pollfd newFd;
    newFd.fd = WaitConnection.GetHandle();
    newFd.events = get_event(ConnectionEvent);
    data.push_back(newFd);
}

void Poll::Remove(const Connection &RemoveConnection)
{
    std::vector<pollfd>::iterator it;
    for(it = data.begin(); it != data.end(); ++it)
        if(it->fd == RemoveConnection.GetHandle()){
            data.erase(it);
            return;
        }   
}

void Poll::Remove(const Connection &RemoveConnection, Event RemoveEvent)
{
    std::vector<pollfd>::iterator it;
    for(it = data.begin(); it != data.end(); ++it)
        if(it->fd == RemoveConnection.GetHandle()){
            it->events = it->events & ~get_event(RemoveEvent);

            if(it->events == 0)
                data.erase(it);
            return;
        }   
}

bool Poll::Check(const Connection &WaitConnection, Event ConnectionEvent)
{
    int16_t mask = get_event(ConnectionEvent);
    
    std::vector<pollfd>::iterator it;
    for(it = data.begin(); it != data.end(); ++it)
        if(it->fd == WaitConnection.GetHandle() && it->revents & mask)                    
            return true;
        
    return false;
}

bool Poll::Wait() throw (Exception)
{     
    int32_t result = SocketLevel::poll_socket(&data[0], data.size(), timeout);
        
    return (result > 0);
}

};