/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include "Exception.h"
#include <Windows.h>
DECLARE_EXCEPTION(CreateLockerException);

class Win32Locker
{
private:
	CRITICAL_SECTION cs;
public:
	void Lock();
	void Unlock();	
	void Create() throw (CreateLockerException);
	void Release();
	Win32Locker(){}
	~Win32Locker(){}
};

typedef Win32Locker Locker;
