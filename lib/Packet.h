#pragma once
#include <vector>
#include <string>
#include <map>
#include <Exception.h>
#include <NetworkAction.h>
#include <NetworkVariable.h>
#include <EnumParser.h>
#include <Serializing.h>

namespace Network
{

enum PacketVariableType
{
    VAR_TYPE_UNKNOWN,
    VAR_TYPE_INT8,
    VAR_TYPE_INT16,
    VAR_TYPE_INT32,
    VAR_TYPE_INT64,
    VAR_TYPE_FLOAT,
    VAR_TYPE_DOUBLE,
    VAR_TYPE_STRING,
    VAR_TYPE_ARRAY_OF_INT8,
    VAR_TYPE_ARRAY_OF_INT16,
    VAR_TYPE_ARRAY_OF_INT32,
    VAR_TYPE_ARRAY_OF_INT64,
    VAR_TYPE_ARRAY_OF_FLOAT,
    VAR_TYPE_ARRAY_OF_DOUBLE,
    VAR_TYPE_ARRAY_OF_STRING,
};

template<typename T> struct GetVariableType{};
template<> struct GetVariableType<int8_t> {static const PacketVariableType Type = VAR_TYPE_INT8;};
template<> struct GetVariableType<int16_t> {static const PacketVariableType Type = VAR_TYPE_INT16;};
template<> struct GetVariableType<int32_t> {static const PacketVariableType Type = VAR_TYPE_INT32;};
template<> struct GetVariableType<int64_t> {static const PacketVariableType Type = VAR_TYPE_INT64;};
template<> struct GetVariableType<float> {static const PacketVariableType Type = VAR_TYPE_FLOAT;};
template<> struct GetVariableType<double> {static const PacketVariableType Type = VAR_TYPE_DOUBLE;};
template<> struct GetVariableType<std::string> {static const PacketVariableType Type = VAR_TYPE_STRING;};
template<> struct GetVariableType<std::vector<int8_t> > {static const PacketVariableType Type = VAR_TYPE_ARRAY_OF_INT8;};
template<> struct GetVariableType<std::vector<int16_t> > {static const PacketVariableType Type = VAR_TYPE_ARRAY_OF_INT16;};
template<> struct GetVariableType<std::vector<int32_t> > {static const PacketVariableType Type = VAR_TYPE_ARRAY_OF_INT32;};
template<> struct GetVariableType<std::vector<int64_t> > {static const PacketVariableType Type = VAR_TYPE_ARRAY_OF_INT64;};
template<> struct GetVariableType<std::vector<float> > {static const PacketVariableType Type = VAR_TYPE_ARRAY_OF_FLOAT;};
template<> struct GetVariableType<std::vector<double> > {static const PacketVariableType Type = VAR_TYPE_ARRAY_OF_DOUBLE;};
template<> struct GetVariableType<std::vector<std::string> > {static const PacketVariableType Type = VAR_TYPE_ARRAY_OF_STRING;};

DECLARE_EXCEPTION(InvalidActionException);
DECLARE_EXCEPTION(InvalidVariableDataTypeException);
DECLARE_EXCEPTION(VariableNotFoundException);

class Packet
{
public:
    class Variable
    {
    private:
        PacketVariableType type;
        std::vector<char> rawData;
    public:
        Variable():type(VAR_TYPE_UNKNOWN){}
        void Init(PacketVariableType Type, const std::vector<char> &Data)
        {
            type = Type;
            rawData = Data;
        }
        template<class TVar, typename Parser>
        void Init(PacketVariableType Type, const TVar &Var)
        {
            type = Type;
            rawData.resize(BinarySerializerSize<TVar>::Get(Var));
            Parser::ToBinary(Var, &rawData[0]); //AAA vector addr
        }
        PacketVariableType GetType() const {return type;}
        const std::vector<char> &GetRawData() const {return rawData;}
        operator int8_t() const throw (Exception);
        operator int16_t() const throw (Exception);
        operator int32_t() const throw (Exception);
        operator int64_t () const throw (Exception);
        operator float () const throw (Exception);
        operator double () const throw (Exception);
        operator std::string() const throw (Exception);
        operator std::vector<int8_t>() const throw (Exception);
        operator std::vector<int16_t>() const throw (Exception);
        operator std::vector<int32_t>() const throw (Exception);
        operator std::vector<int64_t>() const throw (Exception);
        operator std::vector<float>() const throw (Exception);
        operator std::vector<double>() const throw (Exception);
        operator std::vector<std::string>() const throw (Exception);
    };
private:
    ActionId action;
    typedef std::map<VariableId, Variable> VariablesStorage;
    VariablesStorage variables;
public:
    typedef VariablesStorage::const_iterator ConstIterator;
    Packet():action(UNKNOWN_ACTION){}
    explicit Packet(ActionId Action):action(Action){}
    ActionId GetAction() const { return action;}
    void SetAction(ActionId Action){action = Action;}
    void AddVariable(VariableId Id, const Variable &NewVariable);
    template<typename TData>
    void CreateVariable(VariableId Id, const TData &Data)
    {
        Network::Packet::Variable newVar;

        newVar.Init<TData, typename ParserType<TData>::Type >(GetVariableType<TData>::Type, Data);

        AddVariable(Id, newVar);
    }
    bool FindVariable(VariableId Id, Variable &FoundVariable);
    ConstIterator Begin() const {return variables.begin();}
    ConstIterator End() const {return variables.end();}
    const Variable &operator [] (VariableId Id) const throw (Exception);
};

class CommonPackets
{
public:
    static Packet Error(ActionId Code, const std::string &Message);
};

}

template<>
class EnumParser<Network::PacketVariableType>
{
public:
    static Network::PacketVariableType FromInt(int32_t Var) throw (Exception);
};
