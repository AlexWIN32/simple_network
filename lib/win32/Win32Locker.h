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
