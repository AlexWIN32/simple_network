/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <EnumParser.h>

namespace Logging
{
    
enum Level
{     
    EMERG = 0,
    ALERT = 1,
    CRIT = 2,
    ERROR = 3,
    WARNING = 4,
    NOTICE = 5,
    INFO = 6,
    DEBUG = 7,
    TRACE = 8,        
};
    
class MessagesWriter
{
public:
    virtual ~MessagesWriter(){}
    virtual void WriteMessage(const std::string &Message) = 0;
};

class MessagesManager
{
private:
    MessagesManager(){}
    ~MessagesManager();    
    static MessagesManager *instance;    
    typedef std::vector<MessagesWriter*> WritersCollection;
    typedef std::map<Level, WritersCollection> MessagesWritersStorage;        
    MessagesWritersStorage writers;
    WritersCollection allWriters;
public:
    static MessagesManager *GetInstance()
    {
        if(!instance)
            instance = new MessagesManager();
        return instance;
    }
    static void ReleaseInstance()
    {
        delete instance;
        instance = NULL;
    }
    void AddWriter(Level MessageLevel, MessagesWriter *NewWriter);
    void AddMessage(Level MessageLevel, const std::string &Module, const std::string &Message);
};

class Logger
{
private:
    std::string module;
public:
    class InputBuffer
    {
    private:
        Level level;
        std::string module;
        std::ostringstream buffer;
    public:
        InputBuffer(const InputBuffer & ob) : level(ob.level), module(ob.module) {}
        InputBuffer(Level MessageLevel, const std::string &Module) : level(MessageLevel), module(Module){}
        template <class TData>
        InputBuffer &operator << (const TData& Data)
        {
            buffer << Data;
            return *this;
        }
        ~InputBuffer()
        {
            MessagesManager::GetInstance()->AddMessage(level, module, buffer.str());   
        }
    };
    explicit Logger(const std::string &Module):module(Module){}
    InputBuffer operator << (Level level) const { return InputBuffer(level, module); }    
};

class OutputWriter : public MessagesWriter
{
public:
    virtual ~OutputWriter(){}
    virtual void WriteMessage(const std::string &Message);
};

class FileWriter : public MessagesWriter
{
private:
    FILE *file;
    bool useTime;
    int32_t endDayTimestamp;
    std::string path;
public:
    FileWriter():file(NULL), useTime(false){}
    virtual ~FileWriter()
    {
        if(file)
            fclose(file);        
    }
    void Init(const std::string &Path, bool UseTime = false) throw (Exception);
    virtual void WriteMessage(const std::string &Message);
};

}

template <>
class EnumParser<Logging::Level>
{
public:    
    static std::string ToString(Logging::Level Val)
    {
        switch(Val){
            case Logging::EMERG : 
                return "EMERG";
            case Logging::CRIT : 
                return "CRIT";
            case Logging::NOTICE : 
                return "NOTICE";
            case Logging::ALERT : 
                return "ALERT";
            case Logging::ERROR : 
                return "ERROR";
            case Logging::WARNING : 
                return "WARNING";
            case Logging::INFO : 
                return "INFO";
            case Logging::DEBUG : 
                return "DEBUG";
            case Logging::TRACE : 
                return "TRACE";
            default:
                return "";            
        }
    }
};