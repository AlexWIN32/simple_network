/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include "Serializing.h"

Range<int> GetStatementParameters(const std::string &String, char OpenSymbol, char CloseSymbol)
{
	Range<int> vals(-1, -1);
	int cnt = 0;		
	for(size_t i = 0; i < String.size(); i++){
		if(String[i] == OpenSymbol && !cnt++) vals.minVal = (int)i;			
		if(String[i] == CloseSymbol && !--cnt) vals.maxVal = (int)i;					
	}
	return vals;
}
