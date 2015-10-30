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
