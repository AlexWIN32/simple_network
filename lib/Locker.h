/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

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
