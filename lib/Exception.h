#pragma once
#include <string>

struct Exception
{
	std::string msg;
	Exception(const std::string &Msg) : msg(Msg){}
};

#define DECLARE_EXCEPTION(CLASS_NAME) struct CLASS_NAME : public Exception{CLASS_NAME(const std::string &Msg) : Exception(Msg){}};
#define DECLARE_CHILD_EXCEPTION(CLASS_NAME, BASE_CLASS_NAME) struct CLASS_NAME : public BASE_CLASS_NAME{CLASS_NAME(const std::string &Msg) : BASE_CLASS_NAME(Msg){}};

DECLARE_EXCEPTION(IOException)
DECLARE_CHILD_EXCEPTION(ReadException, IOException)
DECLARE_CHILD_EXCEPTION(WriteException, IOException)
