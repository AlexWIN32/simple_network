#pragma once
#include <Exception.h>

class Locker
{
private:
    pthread_mutex_t mutex;
public:
    void Lock();
    void Unlock();  
    void Create() throw (Exception);
    void Release();
    Locker(){}
    ~Locker(){}
};


class LockGuard
{
private:
	Locker &locker;
	LockGuard(const LockGuard &);
	LockGuard& operator = (const LockGuard &);
public:
	explicit LockGuard(Locker &ExtLocker) : locker(ExtLocker) 	
	{
		locker.Lock();
	}
	~LockGuard()
	{
		locker.Unlock();
	}
};
