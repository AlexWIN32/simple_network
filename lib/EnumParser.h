/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

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