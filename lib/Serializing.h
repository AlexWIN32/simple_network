#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <memory.h>
#include "Exception.h"
#include "Vector2.h"
DECLARE_EXCEPTION(FormatException);
DECLARE_EXCEPTION(ArraySizeException);

Range<int> GetStatementParameters(const std::string &String, char OpenSymbol, char CloseSymbol);

template<typename Parser>
class SerializerTools
{
public:
	typedef std::pair<typename Parser::ValueType, typename Parser::ValueType> Pair;
	static Pair PairFromString(const std::string &String, char OpenSymbol, char CloseSymbol) throw (FormatException)
	{
		Range<int> params = GetStatementParameters(String, OpenSymbol, CloseSymbol);

		if(params.minVal == -1 || params.maxVal == -1) throw FormatException("invalid string " + String);

		int cnt = 0;
		bool isSecondVal = false;
		std::string firstVal = "", secondVal = "";

		std::string values = String.substr(params.minVal + 1, params.maxVal - params.minVal - 1);
		for(size_t i = 0; i < values.size(); i++){
			if(values[i] == ' ') continue;
			if(values[i] == '[' || values[i] == '(' || values[i] == '{' || values[i] == '<') cnt++;
			if(values[i] == ']' || values[i] == ')' || values[i] == '}' || values[i] == '>') cnt--;
			if(values[i] == ',' && !cnt) {isSecondVal = true; continue;}

			(isSecondVal ? secondVal : firstVal) += values[i];
		}

		if(firstVal == "" || secondVal == "") throw FormatException("invalid string " + String);

		return std::make_pair(Parser::FromString(firstVal), Parser::FromString(secondVal));
	}
	static std::string PairToString(const Pair &DataPair, char OpenSymbol, char CloseSymbol) throw (FormatException)
	{
		std::ostringstream sstrm;
		sstrm << OpenSymbol << Parser::ToString(DataPair.first) << ',' << Parser::ToString(DataPair.second) << CloseSymbol;
		return sstrm.str();
	}
	static Pair PairFromBinary(const char* Data)
	{
		Pair outPair;
		outPair.first = Parser::FromBinary(Data);
		outPair.second = Parser::FromBinary(Data + sizeof(typename Parser::ValueType));
		return outPair;
	}
	static void PairToBinary(const Pair &DataPair, char* Data)
	{
		Parser::ToBinary(DataPair.first, Data);
		Parser::ToBinary(DataPair.second, Data + sizeof(typename Parser::ValueType));
 	}
	typedef std::vector<typename Parser::ValueType> Array;
	static Array ArrayFromString(const std::string &String, char OpenSymbol, char CloseSymbol, char Delimiter) throw (FormatException)
	{
		Range<int> params = GetStatementParameters(String, OpenSymbol, CloseSymbol);

		if(params.minVal == -1 || params.maxVal == -1) throw FormatException("invalid string " + String);

		int cnt = 0;
		std::string tmpVar = "";
		Array outData;
		std::string values = String.substr(params.minVal + 1, params.maxVal - params.minVal - 1);
		for(size_t i = 0; i < values.size(); i++){
			if(values[i] == ' ') continue;
			if(values[i] == '[' || values[i] == '(' || values[i] == '{' || values[i] == '<') cnt++;
			if(values[i] == ']' || values[i] == ')' || values[i] == '}' || values[i] == '>') cnt--;
			if(values[i] == ',' && !cnt && tmpVar != "") {
				outData.push_back(Parser::FromString(tmpVar));
				tmpVar = "";
				continue;
			}
			tmpVar += values[i];
		}
		if(tmpVar != "")outData.push_back(Parser::FromString(tmpVar));

		return outData;
	}
	static std::string ArrayToString(const Array &Data, char OpenSymbol, char CloseSymbol, char Delimiter) throw (FormatException)
	{
		std::ostringstream sstrm;
		sstrm << OpenSymbol;
		for(size_t i = 0, size = Data.size(); i < size; i++){
			sstrm << Parser::ToString(Data[i]);
			if(i < size - 1)sstrm << ",";
		}
		sstrm << CloseSymbol;
		return sstrm.str();
	}
	static Array ArrayFromBinary(const char* Data)
	{
		Array outArray;
		const size_t arrSize = *reinterpret_cast<const size_t*>(Data);
		Data += sizeof(size_t);
		for(size_t i = 0; i < arrSize; i++){
			outArray.push_back(Parser::FromBinary(Data));
			Data += sizeof(typename Parser::ValueType);
		}
		return outArray;
	}
	static void ArrayToBinary(const Array &DataArr, char* Data)
	{
		*reinterpret_cast<size_t*>(Data) = DataArr.size();
		Data += sizeof(size_t);
		for(size_t i = 0, size = DataArr.size(); i < size; i++){
			Parser::ToBinary(DataArr[i], Data);
			Data += sizeof(typename Parser::ValueType);
		}
 	}
};

template<typename T>
class NumberConverter
{
public:
	static T FromString(const std::string &String) throw (FormatException)
	{
		Range<int> params = GetStatementParameters(String, '<', '>');

		if(params.minVal == -1 && params.maxVal == -1){
			std::string lowerCase = String;
			std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), ::tolower);
			if(lowerCase.find("0x") != std::string::npos){
				long int outVar;
				std::stringstream ss;
				ss << std::hex << lowerCase;
				ss >> outVar;
				return static_cast<T>(outVar);
			}

			return static_cast<T>(atoi(String.c_str()));
		}else if(params.minVal == -1 || params.maxVal == -1)
			throw FormatException("invalid random value syntax for sring " + String);

		std::string minValStr, maxValStr;

		bool isSecondVal = false;
		std::string values = String.substr(params.minVal + 1, params.maxVal - params.minVal - 1);
		for(size_t i = 0; i < values.size(); i++){
			if(values[i] == ',') {isSecondVal = true; continue;}

			(isSecondVal ? maxValStr : minValStr) += values[i];
		}

		int minVal = atoi(minValStr.c_str()), maxVal = atoi(maxValStr.c_str());
		if(minVal > maxVal) throw FormatException("invalid values for sring " + String);

		return static_cast<T>( minVal + (rand() % (maxVal - minVal)));
	}
};

template<>
class NumberConverter<float>
{
public:
	static float FromString(const std::string &String)
	{
		Range<int> params = GetStatementParameters(String, '<', '>');

		if(params.minVal == -1 && params.maxVal == -1)
			return (float)atof(String.c_str());
		else if(params.minVal == -1 || params.maxVal == -1)
			throw FormatException("invalid random value syntax for sring " + String);

		std::string minValStr, maxValStr;

		bool isSecondVal = false;
		std::string values = String.substr(params.minVal + 1, params.maxVal - params.minVal - 1);
		for(size_t i = 0; i < values.size(); i++){
			if(values[i] == ',') {isSecondVal = true; continue;}

			(isSecondVal ? maxValStr : minValStr) += values[i];
		}

		float minVal = atof(minValStr.c_str()), maxVal = atof(maxValStr.c_str());
		if(minVal > maxVal) throw FormatException("invalid values for sring " + String);

		return minVal + ((maxVal - minVal) * (static_cast<float>(rand() % 100) * 0.01f));
	}
};

template<>
class NumberConverter<bool>
{
private:
	static bool ParseVal(const std::string &String)
	{
		std::string str = String;
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);

		bool outVal = false;
		if(str == "true" || str == "yes" || str == "1")
			outVal = true;
		else if(str == "false" || str == "no" || str == "0")
			outVal = false;
		else throw FormatException("Invalid value " + String);

		return outVal;
	}
public:
	static bool FromString(const std::string &String)
	{
		Range<int> params = GetStatementParameters(String, '<', '>');

		if(params.minVal == -1 && params.maxVal == -1){
			return ParseVal(String);
		}else if(params.minVal == -1 || params.maxVal == -1)
			throw FormatException("invalid random value syntax for sring " + String);

		std::string minValStr, maxValStr;

		bool isSecondVal = false;
		std::string values = String.substr(params.minVal + 1, params.maxVal - params.minVal - 1);

		values.erase(remove_if(values.begin(), values.end(), isspace), values.end());

		for(size_t i = 0; i < values.size(); i++){
			if(values[i] == ',') {isSecondVal = true; continue;}
			(isSecondVal ? maxValStr : minValStr) += values[i];
		}

		ParseVal(minValStr);
		ParseVal(maxValStr);

		return (bool)(rand() % 2);
	}
};

template<typename T, class Converter = NumberConverter<T> >
class NumericParser
{
public:
	typedef T ValueType;
	static T FromString(const std::string &String) throw (FormatException)
	{
		Range<int> params = GetStatementParameters(String, '{', '}');
		if(params.minVal == -1 && params.maxVal == -1)
			return Converter::FromString(String);
		else if(params.minVal == -1 || params.maxVal == -1)
			throw FormatException("invalid random value syntax for sring " + String);

		std::vector<T> arr = SerializerTools<NumericParser<T, Converter> >::ArrayFromString(String, '{', '}', ',');
		return arr[rand() % arr.size()];
	}
	static std::string ToString(const T &Value)
	{
		std::ostringstream sstrm;
		sstrm << Value;
		return sstrm.str();
	}
	static T FromBinary(const char* Data)
	{
		return *((T*)Data);
	}
	static void ToBinary(const T &Value, char *OutData)
	{
		//for(int i = 0; i < sizeof(T); i++)OutData[i] = ((char*)&Value)[i];
		memcpy(OutData, (char*)&Value, sizeof(T));
	}
};
typedef NumericParser<int> IntParser;
typedef NumericParser<unsigned short> USParser;
typedef NumericParser<unsigned char> UCharParser;
typedef NumericParser<float> FloatParser;

class StringParser
{
public:
	typedef std::string ValueType;
	static std::string FromString(const std::string &String) throw (FormatException)
	{
		Range<int> params = GetStatementParameters(String, '{', '}');
		if(params.minVal == -1 && params.maxVal == -1)
			return String;
		else if(params.minVal == -1 || params.maxVal == -1)
			throw FormatException("invalid random value syntax for sring " + String);

		std::vector<std::string> arr = SerializerTools<StringParser>::ArrayFromString(String, '{', '}', ',');
		return arr[rand() % arr.size()];
	}
	static std::string ToString(const std::string &String)
	{
		return String;
	}
	static std::string FromBinary(const char* Data)
	{
		std::string outString;
		size_t size = *((size_t*)Data); Data += sizeof(size_t);
		for(size_t i = 0; i < size; i++)
			outString += Data[i];
		return outString;
	}
	static void ToBinary(const std::string &Value, char *OutData)
	{
		*((size_t*)OutData) = Value.length(); OutData += sizeof(size_t);
		for(size_t i = 0; i <  Value.length(); i++)
			OutData[i] = Value[i];
	}
};

template<typename Parser, typename VectorType = BasePoint2<typename Parser::ValueType> >
class BasePoint2Parser
{
public:
	typedef VectorType ValueType;
	static VectorType FromString(const std::string &String) throw (FormatException)
	{
		std::string values = String;
		values.erase(remove_if(values.begin(), values.end(), isspace), values.end());

		Range<int> params = GetStatementParameters(values, '{', '}');
		if(params.minVal == 0 && params.maxVal == values.length() - 1){
			std::vector<ValueType> arr = SerializerTools<BasePoint2Parser<Parser, VectorType> >::ArrayFromString(values, '{', '}', ',');
			return arr[rand() % arr.size()];
		}else if((params.minVal != -1 && params.maxVal == -1) ||
			     (params.minVal == -1 && params.maxVal != -1))
			throw FormatException("invalid random value syntax for sring " + String);

		typename SerializerTools<Parser>::Pair pair = SerializerTools<Parser>::PairFromString(String,'(', ')');
		return VectorType(pair.first, pair.second);
	}
	static std::string ToString(const VectorType &Data) throw (FormatException)
	{
		typename SerializerTools<Parser>::Pair pair(Data.x, Data.y);
		return SerializerTools<Parser>::PairToString(pair, '(', ')');
	}
	static VectorType FromBinary(const char *RawData)
	{
		typename SerializerTools<Parser>::Pair pair = SerializerTools<Parser>::PairFromBinary(RawData);
		return VectorType(pair.first, pair.second);
	}
	static void ToBinary(const VectorType &Data, char *RawData)
	{
		typename SerializerTools<Parser>::Pair pair(Data.x, Data.y);
		SerializerTools<Parser>::PairToBinary(pair, RawData);
	}
};

template<typename Parser, typename SizeType = Size<typename Parser::ValueType> >
class SizeParser
{
public:
	typedef SizeType ValueType;
	static SizeType FromString(const std::string &String)
	{
		std::string values = String;
		values.erase(remove_if(values.begin(), values.end(), isspace), values.end());

		Range<int> params = GetStatementParameters(values, '{', '}');
		if(params.minVal == 0 && params.maxVal == values.length() - 1){
			std::vector<ValueType> arr = SerializerTools<SizeParser<Parser, SizeType> >::ArrayFromString(values, '{', '}', ',');
			return arr[rand() % arr.size()];
		}else if((params.minVal != -1 && params.maxVal == -1) ||
			     (params.minVal == -1 && params.maxVal != -1))
			throw FormatException("invalid random value syntax for sring " + String);

		typename SerializerTools<Parser>::Pair pair = SerializerTools<Parser>::PairFromString(String, '(', ')');
		return SizeType(pair.first, pair.second);
	}
	static std::string ToString(const SizeType &Data)
	{
		return SerializerTools<Parser>::PairToString(SerializerTools<Parser>::Pair(Data.width, Data.height), '(', ')');
	}
	static SizeType FromBinary(const char *RawData)
	{
		typename SerializerTools<Parser>::Pair pair = SerializerTools<Parser>::PairFromBinary(RawData);
		return SizeType(pair.first, pair.second);
	}
	static void ToBinary(const SizeType &Data, char *RawData)
	{
        typename SerializerTools<Parser>::Pair pair(Data.width, Data.height);
		SerializerTools<Parser>::PairToBinary(pair, RawData);
	}
};

template<typename Parser, typename RangeType = Range<typename Parser::ValueType> >
class RangeParser
{
public:
	typedef RangeType ValueType;
	static RangeType FromString(const std::string &String) throw (FormatException)
	{
		std::string values = String;
		values.erase(remove_if(values.begin(), values.end(), isspace), values.end());

		Range<int> params = GetStatementParameters(values, '{', '}');
		if(params.minVal == 0 && params.maxVal == values.length() - 1){
			std::vector<ValueType> arr = SerializerTools<RangeParser<Parser, RangeType> >::ArrayFromString(values, '{', '}', ',');
			return arr[rand() % arr.size()];
		}else if((params.minVal != -1 && params.maxVal == -1) ||
			     (params.minVal == -1 && params.maxVal != -1))
			throw FormatException("invalid random value syntax for sring " + String);

		typename SerializerTools<Parser>::Pair pair = SerializerTools<Parser>::PairFromString(String, '[', ']');
		return RangeType(pair.first, pair.second);
	}
	static std::string ToString(const RangeType &Range)
	{
		return SerializerTools<Parser>::PairToString(SerializerTools<Parser>::Pair(Range.minVal, Range.maxVal), '[', ']');
	}
	static RangeType FromBinary(const char *Data)
	{
		typename SerializerTools<Parser>::Pair pair = SerializerTools<Parser>::PairFromBinary(Data);
		return RangeType(pair.first, pair.second);
	}
	static void ToBinary(const RangeType &Range, char *Data)
	{
		SerializerTools<Parser>::PairToBinary(SerializerTools<Parser>::Pair(Range.minVal, Range.maxVal), Data);
	}
};
template<typename Parser>
class ArrayParser
{
public:
	typedef typename SerializerTools<Parser>::Array ValueType;
	static ValueType FromString(const std::string &String)
	{
		std::string values = String;
		values.erase(remove_if(values.begin(), values.end(), isspace), values.end());

		Range<int> params = GetStatementParameters(values, '{', '}');
		if(params.minVal == 0 && params.maxVal == values.length() - 1){
			std::vector<ValueType> arr = SerializerTools<ArrayParser<Parser> >::ArrayFromString(values, '{', '}', ',');
			return arr[rand() % arr.size()];
		}else if((params.minVal != -1 && params.maxVal == -1) ||
			     (params.minVal == -1 && params.maxVal != -1))
			throw FormatException("invalid random value syntax for sring " + String);

		return SerializerTools<Parser>::ArrayFromString(String, '(', ')', ',');
	}
	static std::string ToString(const ValueType &Array)
	{
		return SerializerTools<Parser>::ArrayToString(Array, '(', ')', ',');
	}
	static ValueType FromBinary(const char *Data)
	{
		return SerializerTools<Parser>::ArrayFromBinary(Data);
	}
	static void ToBinary(const ValueType &Array, char *Data)
	{
		return SerializerTools<Parser>::ArrayToBinary(Array, Data);
	}
};

template<typename Parser>
class ColorParser
{
public:
	struct Channels
	{
		typename Parser::ValueType B, G, R, A;
	};
	static Channels FromString(const std::string &String)
	{
		typename ArrayParser<Parser>::ValueType arr = ArrayParser<Parser>::FromString(String);
		if(arr.size() != 4) throw FormatException("invalid color syntax for sring " + String);

		Channels outData;
		outData.B = arr[0];
		outData.G = arr[1];
		outData.R = arr[2];
		outData.A = arr[3];
		return outData;
	}
	static std::string ToString(const Channels &Color)
	{
		typename SerializerTools<Parser>::Array arr;
		arr.push_back(Color.B);
		arr.push_back(Color.G);
		arr.push_back(Color.R);
		arr.push_back(Color.A);
		return SerializerTools<Parser>::ArrayToString(arr, '(', ')', ',');
	}
	static Channels FromBinary(const char *Data)
	{
		Channels outData;
		outData.B = Parser::FromBinary(Data); Data += sizeof(Parser::ValueType);
		outData.G = Parser::FromBinary(Data); Data += sizeof(Parser::ValueType);
		outData.R = Parser::FromBinary(Data); Data += sizeof(Parser::ValueType);
		outData.A = Parser::FromBinary(Data); Data += sizeof(Parser::ValueType);
		return outData;
	}
	static void ToBinary(const Channels &Color, char *Data)
	{
		Parser::ToBinary(Color.B, Data); Data += sizeof(Parser::ValueType);
		Parser::ToBinary(Color.G, Data); Data += sizeof(Parser::ValueType);
		Parser::ToBinary(Color.R, Data); Data += sizeof(Parser::ValueType);
		Parser::ToBinary(Color.A, Data); Data += sizeof(Parser::ValueType);
	}
};

typedef RangeParser<USParser> RangeUSParser;
typedef RangeParser<FloatParser> RangeFParser;

typedef BasePoint2Parser<IntParser> Point2Parser;
typedef BasePoint2Parser<FloatParser> Point2FParser;
typedef BasePoint2Parser<FloatParser, Vector2> Vector2Parser;

typedef SizeParser<USParser> SizeUSParser;
typedef SizeParser<FloatParser> SizeFParser;

typedef ColorParser<UCharParser> ColorUCharParser;

template<typename T>
class BinarySerializerSize
{
public:
    static size_t Get(T Var){ return sizeof(T);}
};

template<>
class BinarySerializerSize<std::string>
{
public:
    static size_t Get(const std::string &Var){ return Var.length() + sizeof(size_t);}
};

template<typename T>
struct BinarySerializerSize<std::vector<T> >
{
    static size_t Get(const std::vector<T> &Var){ return Var.size() + (sizeof(T) * Var.size());}
};

template<typename T>
struct ParserType
{
    typedef NumericParser<T> Type;
};

template<typename T>
struct ParserType<std::vector<T> >
{
    typedef ArrayParser<T> Type;
};

template<>
struct ParserType<std::string>
{
    typedef StringParser Type;
};