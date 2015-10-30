#include <Locker.h>

DECLARE_EXCEPTION(UnixLockerCreationException)

void Locker::Create() throw (Exception)
{
    if(pthread_mutex_init(&mutex, NULL) == -1)
        throw UnixLockerCreationException("");
}

void Locker::Lock()
{
    pthread_mutex_lock(&mutex);
}

void Locker::Unlock()
{
    pthread_mutex_unlock(&mutex);
}

void Locker::Release()
{
    pthread_mutex_destroy(&mutex);
}
