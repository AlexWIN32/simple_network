/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <iostream>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <getopt.h>
#include <Server.h>
#include <MessagesProcessor.h>
#include <Logger.h>
#include <SimpleTcpConnection.h>
#include <SSLConnection.h>

static Logging::Logger logger("Main routine");

static void usage()
{
    std::cout <<
    "usage: server" << std::endl <<
    "       -p   port" << std::endl <<
    "      [-l]  log file path" << std::endl <<
    "      [-s]  enable SSL " << std::endl <<
    "      [-k]  private key file path " << std::endl <<
    "      [-b]  sertificate file path " << std::endl;
}

const Network::SSLContextKey ServerContext = 1;

int main(int argc, char **argv)
{
    std::string port, logFilePath;
    std::string pKeyFilePath, certFilePath;
    std::string pkeyPassPhrase;

    bool enableSSl = false;
    bool usePassPhrase = false;
    Network::PrivateKey pkey;
    Network::X509Certificate cert;

    char c;
    while ((c = getopt( argc, argv, "p:v?sk:b:a" )) != EOF)
        switch (c) {
            case 'p':
                port = optarg;
                break;
            case 'l':
                logFilePath = optarg;
                break;
            case 's':
                enableSSl = true;
                break;
            case 'k':
                pKeyFilePath = optarg;
                break;
            case 'b':
                certFilePath = optarg;
                break;
            case 'a':
                usePassPhrase = true;
                break;
            case '?':
            case 'v':
            default:
                usage();
                exit(0);
        }

    if(port == ""){
        usage();
        exit(0);
    }

    if(usePassPhrase)
        pkeyPassPhrase = getpass("enter passphrase:");

    int16_t portVal = static_cast<int16_t>(atoi(port.c_str()));

    if(enableSSl && (pKeyFilePath == "" || certFilePath == "")){
        std::cout << "specify private key file and certificate file path";
        exit(0);
    }

    Logging::MessagesWriter *writer = NULL;
    if(logFilePath != ""){
        Logging::FileWriter *fileWriter = new Logging::FileWriter();
        try{
            fileWriter->Init(logFilePath);
        }catch(const Exception &ex){
            std::cout << "cant open log file: " << ex.msg <<std::endl;
            delete fileWriter;
            return 1;
        }

        writer = fileWriter;
    }else
        writer = new Logging::OutputWriter();

    Logging::MessagesManager::GetInstance()->AddWriter(Logging::INFO, writer);
    Logging::MessagesManager::GetInstance()->AddWriter(Logging::DEBUG, writer);
    Logging::MessagesManager::GetInstance()->AddWriter(Logging::ERROR, writer);

    MessagesProcessor processor;
    MessagesProcessor::NameCheckQueryProcessor nameCheck(&processor);

    Network::Server server;
    server.AddManager(&processor);
    server.AddManager(&nameCheck);

    try{

        Network::ServerConnection *conn;
        if(enableSSl){
            Network::InitSSL();

            pkey.LoadFromFile(pKeyFilePath, Network::SSL_DATA_TYPE_PEM, pkeyPassPhrase);
            cert.LoadFromFile(certFilePath, Network::SSL_DATA_TYPE_PEM);

            Network::SSLContext *context = new Network::SSLContext();
            context->Init(Network::SSL_CONTEXT_TLS_VER_1_2);
            context->UsePrivateKey(pkey, cert);

            Network::SSLContextStorage::GetInstance()->Add(ServerContext, context);

            Network::SSLServerConnection *sslConn = new Network::SSLServerConnection();
            sslConn->Init(portVal, 10, ServerContext);
            conn = sslConn;
        }else{
            Network::SimpleTcpServerConnection *serverConn = new Network::SimpleTcpServerConnection();
            serverConn->Init(portVal, 10);
            conn = serverConn;
        }

        server.Init(10, conn);
        logger << Logging::INFO << "server started";
        server.Run();
    }catch(const Exception &ex){
        logger << Logging::ERROR << ex.msg;
    }

    Logging::MessagesManager::ReleaseInstance();

    if(enableSSl)
        Network::SSLContextStorage::ReleaseInstance();
}