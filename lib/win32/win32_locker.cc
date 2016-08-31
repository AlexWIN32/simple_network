/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

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
