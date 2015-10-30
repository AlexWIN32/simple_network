#pragma once 
#include <algorithm>
#include <Exception.h>

DECLARE_EXCEPTION(EnumException);

template <class TEnum>
class EnumParser
{
public:
	static TEnum FromString(const std::string &Val) throw (Exception);
	static std::string ToString(TEnum Val, bool IsNumeric);
};