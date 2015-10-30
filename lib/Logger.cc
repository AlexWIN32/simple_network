#include <Logger.h>
#include <iostream>
#include <time.h>

namespace Logging
{

MessagesManager *MessagesManager::instance = NULL;

static std::string get_time_str()
{            
    time_t rawtime;
    time(&rawtime);
    
    struct tm * timeinfo = localtime(&rawtime);
    
    const size_t timeBuffer = 80;    
    char buffer[timeBuffer];
    strftime(buffer, timeBuffer, "%D %T", timeinfo);

    return buffer;
}

MessagesManager::~MessagesManager()
{
    WritersCollection::const_iterator ci;
    for(ci = allWriters.begin(); ci != allWriters.end(); ++ci)
        delete *ci;
}

void MessagesManager::AddWriter(Level MessageLevel, MessagesWriter *NewWriter)
{
    WritersCollection &collection = writers[MessageLevel];

    if(std::find(collection.begin(), collection.end(), NewWriter) != collection.end())
        return;

    collection.push_back(NewWriter);

    if(std::find(allWriters.begin(), allWriters.end(), NewWriter) == allWriters.end())
        allWriters.push_back(NewWriter);
}

void MessagesManager::AddMessage(Level MessageLevel, const std::string &Module, const std::string &Message)
{
    MessagesWritersStorage::const_iterator ci = writers.find(MessageLevel);
    if(ci == writers.end())
        return;

    const WritersCollection &collection = ci->second;
    if(!collection.size())
        return;

    std::ostringstream sstrm;
    sstrm << get_time_str() << " " << EnumParser<Level>::ToString(MessageLevel) <<
    " " << Module << ": " << Message << std::endl;

    WritersCollection::const_iterator ci2;
    for(ci2 = collection.begin(); ci2 != collection.end(); ++ci2)
        (*ci2)->WriteMessage(sstrm.str());
}

void OutputWriter::WriteMessage(const std::string &Message)
{
    std::cout << Message;
}

static std::string get_date_file_name(int32_t &EndDayTimestamp)
{
    time_t rawtime;
    time(&rawtime);

    struct tm * ptm = localtime(&rawtime);
    ptm->tm_mday += 1;
    ptm->tm_hour = ptm->tm_min = ptm->tm_sec = 0;

    EndDayTimestamp = mktime(ptm);

    char buff[80];
    strftime(buff, 80, "%F", ptm);

    return buff;
}

void FileWriter::Init(const std::string &Path, bool UseTime) throw (Exception)
{
    useTime = UseTime;

    std::string filePath = Path;

    if(useTime){
        path = Path;
        filePath = Path + "/" + get_date_file_name(endDayTimestamp);
    }

    file = fopen(filePath.c_str(), "a");
    if(!file)
        throw IOException("cant open " + filePath);
}

void FileWriter::WriteMessage(const std::string &Message)
{
    if(useTime){
        time_t rawtime;
        time(&rawtime);

        if(rawtime > endDayTimestamp){

            fclose(file);

            std::string newFilePath = path + "/" + get_date_file_name(endDayTimestamp);
            file = fopen(newFilePath.c_str(), "a");
        }
    }

    if(file)
        fwrite(Message.c_str(), sizeof(char), Message.length(), file);
}
    
}
