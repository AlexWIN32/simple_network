#pragma once
#include <Connection.h>
#include <Logger.h>

namespace Network
{

class PacketProcessor
{
private:
    PacketProcessor();
    PacketProcessor(const PacketProcessor&);
public:
    template<class TSocket, typename Func>
    static Packet Read(bool &IsClosed, TSocket Socket, Func ReadFunc) throw (Exception)
    {
        int32_t actionVal;
        int32_t len = ReadFunc(Socket, &actionVal, sizeof(int32_t));
        if(len == 0){
            IsClosed = true;
            return Packet();
        }

        ActionId action = static_cast<ActionId>(actionVal);

        Packet newPacket(action);

        int8_t varsLen;
        ReadFunc(Socket, &varsLen, sizeof(int8_t));

        for(int8_t v = 0; v < varsLen; v++){
            VariableId varId;
            ReadFunc(Socket, &varId, sizeof(VariableId));

            int32_t varTypeVal;
            ReadFunc(Socket, &varTypeVal, sizeof(int32_t));
            Network::PacketVariableType varType = EnumParser<Network::PacketVariableType>::FromInt(varTypeVal);

            int32_t dataLen;
            ReadFunc(Socket, &dataLen, sizeof(int32_t));
            if(!dataLen)
                throw IOException("Empty data for "
                                "action" + NumericParser<ActionId>::ToString(action) + " "
                                "variable " + IntParser::ToString(varTypeVal));

            std::vector<char> data(dataLen);
            ReadFunc(Socket, &data[0], dataLen); //AAA vector addr

            Packet::Variable newVar;
            newVar.Init(varType, data);
            newPacket.AddVariable(varId, newVar);
        }

        return newPacket;
    }
    template<class TSocket, typename Func>
    static void Write(TSocket Socket, const Packet &DataPacket, Func WriteFunc) throw (Exception)
    {
        int32_t actionVal = static_cast<int32_t>(DataPacket.GetAction());
        WriteFunc(Socket, &actionVal, sizeof(int32_t));

        int8_t varsLen = static_cast<int8_t>(std::distance(DataPacket.Begin(), DataPacket.End()));
        WriteFunc(Socket, &varsLen, sizeof(int8_t));

        Packet::ConstIterator ci;
        for(ci = DataPacket.Begin(); ci != DataPacket.End(); ++ci){
            VariableId varId = ci->first;
            WriteFunc(Socket, &varId, sizeof(VariableId));

            const Packet::Variable &var = ci->second;
            int32_t varType = static_cast<int32_t>(var.GetType());
            WriteFunc(Socket, &varType, sizeof(int32_t));

            const std::vector<char> &rawData = var.GetRawData();
            int32_t dataLen = rawData.size();
            if(!dataLen)
                throw IOException("Empty data for "
                                "action " + NumericParser<ActionId>::ToString(actionVal) + " "
                                "variable " + IntParser::ToString(varType));

            WriteFunc(Socket, &dataLen, sizeof(int32_t));
            WriteFunc(Socket, &rawData[0], dataLen); //AAA vector addr
        }
    }
};

}