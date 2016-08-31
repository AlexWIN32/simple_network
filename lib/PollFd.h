/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once

#ifdef WIN32

struct pollfd
{
    int fd;
    short events;
    short revents;
};

#else
#include <sys/poll.h>
#endif
