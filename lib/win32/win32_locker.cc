#include "Win32Locker.h"

void Win32Locker::Create() throw (CreateLockerException)
{
    InitializeCriticalSection(&cs);           
}

void Win32Locker::Lock()
{	
    EnterCriticalSection(&cs);     
}

void Win32Locker::Unlock()
{
	LeaveCriticalSection(&cs);
}

void Win32Locker::Release()
{
	DeleteCriticalSection(&cs);
}
