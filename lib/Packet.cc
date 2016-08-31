/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <Packet.h>
#include <Serializing.h>

namespace Network
{

template< class Type, Network::PacketVariableType VarType, typename Parser = NumericParser<Type> >
class Caster
{
public:
    static Type type_cast(const char *data, Network::PacketVariableType TestType)
    {
        if(TestType != VarType)
            throw InvalidVariableDataTypeException("Wrong data type");

        return Parser::FromBinary(data);
    }
};

Packet::Variable::operator int8_t() const throw (Exception)
{
    return Caster<int8_t, VAR_TYPE_INT8>::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator int16_t() const throw (Exception)
{
    return Caster<int16_t, VAR_TYPE_INT16>::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator int32_t() const throw (Exception)
{
    return Caster<int32_t, VAR_TYPE_INT32>::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator int64_t () const throw (Exception)
{
    return Caster<int64_t, VAR_TYPE_INT64>::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator float () const throw (Exception)
{
    return Caster<float, VAR_TYPE_FLOAT>::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator double () const throw (Exception)
{
    return Caster<double, VAR_TYPE_DOUBLE>::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator std::string() const throw (Exception)
{
    return Caster<std::string, VAR_TYPE_STRING, StringParser>::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator std::vector<int8_t>() const throw (Exception)
{
    return Caster<std::vector<int8_t>,
                     VAR_TYPE_ARRAY_OF_INT8,
                     ArrayParser<NumericParser<int8_t> > >::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator std::vector<int16_t>() const throw (Exception)
{
    return Caster<std::vector<int16_t>,
                     VAR_TYPE_ARRAY_OF_INT16,
                     ArrayParser<NumericParser<int16_t> > >::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator std::vector<int32_t>() const throw (Exception)
{
    return Caster<std::vector<int32_t>,
                     VAR_TYPE_ARRAY_OF_INT32,
                     ArrayParser<NumericParser<int32_t> > >::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator std::vector<int64_t>() const throw (Exception)
{
    return Caster<std::vector<int64_t>,
                     VAR_TYPE_ARRAY_OF_INT64,
                     ArrayParser<NumericParser<int64_t> > >::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator std::vector<float>() const throw (Exception)
{
    return Caster<std::vector<float>,
                     VAR_TYPE_ARRAY_OF_FLOAT,
                     ArrayParser<NumericParser<float> > >::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator std::vector<double>() const throw (Exception)
{
    return Caster<std::vector<double>,
                     VAR_TYPE_ARRAY_OF_DOUBLE,
                     ArrayParser<NumericParser<double> > >::type_cast(&rawData[0], type); //AAA Vector addr
}

Packet::Variable::operator std::vector<std::string>() const throw (Exception)
{
    return Caster<std::vector<std::string>,
                     VAR_TYPE_ARRAY_OF_STRING,
                     ArrayParser<StringParser> >::type_cast(&rawData[0], type); //AAA Vector addr
}

void Packet::AddVariable(VariableId Id, const Variable &NewVariable)
{
    variables[Id] = NewVariable;
}

bool Packet::FindVariable(VariableId Id, Variable &FoundVariable)
{
    VariablesStorage::const_iterator ci = variables.find(Id);
    if(ci == variables.end())
        return false;

    FoundVariable = ci->second;
    return true;
}

const Packet::Variable &Packet::operator [] (VariableId Id) const throw (Exception)
{
    VariablesStorage::const_iterator ci = variables.find(Id);
    if(ci == variables.end())
        throw VariableNotFoundException("variable " + NumericParser<VariableId>::ToString(Id) + " not found");

    return ci->second;
}

Packet CommonPackets::Error(ActionId Code, const std::string &Message)
{
    Packet packet(ACTION_ERROR);
    packet.CreateVariable(VAR_ERROR_CODE, Code);
    packet.CreateVariable(VAR_ERROR_STRING, Message);
    return packet;
}

}

Network::PacketVariableType EnumParser<Network::PacketVariableType>::FromInt(int32_t Val) throw (Exception)
{
    if(Val == Network::VAR_TYPE_INT8)
        return Network::VAR_TYPE_INT8;
    else if(Val == Network::VAR_TYPE_INT8)
        return Network::VAR_TYPE_INT8;
    else if(Val == Network::VAR_TYPE_INT16)
        return Network::VAR_TYPE_INT16;
    else if(Val == Network::VAR_TYPE_INT32)
        return Network::VAR_TYPE_INT32;
    else if(Val == Network::VAR_TYPE_INT64)
        return Network::VAR_TYPE_INT64;
    else if(Val == Network::VAR_TYPE_FLOAT)
        return Network::VAR_TYPE_FLOAT;
    else if(Val == Network::VAR_TYPE_DOUBLE)
        return Network::VAR_TYPE_DOUBLE;
    else if(Val == Network::VAR_TYPE_STRING)
        return Network::VAR_TYPE_STRING;
    else if(Val == Network::VAR_TYPE_ARRAY_OF_INT8)
        return Network::VAR_TYPE_ARRAY_OF_INT8;
    else if(Val == Network::VAR_TYPE_ARRAY_OF_INT16)
        return Network::VAR_TYPE_ARRAY_OF_INT16;
    else if(Val == Network::VAR_TYPE_ARRAY_OF_INT32)
        return Network::VAR_TYPE_ARRAY_OF_INT32;
    else if(Val == Network::VAR_TYPE_ARRAY_OF_INT64)
        return Network::VAR_TYPE_ARRAY_OF_INT64;
    else if(Val == Network::VAR_TYPE_ARRAY_OF_FLOAT)
        return Network::VAR_TYPE_ARRAY_OF_FLOAT;
    else if(Val == Network::VAR_TYPE_ARRAY_OF_DOUBLE)
        return Network::VAR_TYPE_ARRAY_OF_DOUBLE;
    else if(Val == Network::VAR_TYPE_ARRAY_OF_STRING)
        return Network::VAR_TYPE_ARRAY_OF_STRING;
    else
        throw EnumException("Invalid variable type");
}