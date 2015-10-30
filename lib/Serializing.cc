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
