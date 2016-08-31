/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <DialogWindow.h>
#include <Client.h>
#include <SimpleTcpConnection.h>
#include <SSLConnection.h>
#include <ClientMessagesProcessor.h>
#include <QtCore>
#include <QtGui>
#include <thread>
#include <iostream>
#include <getopt.h>
#include <Logger.h>

static void usage()
{
    std::cout <<
    "usage: client" << std::endl <<
    "      [-s]  enable SSL " << std::endl;
}

const Network::SSLContextKey ClientContext = 1;

int main(int argc, char* argv[])
{
    bool enableSSL = false;

    char c;
    while ((c = getopt( argc, argv, "?s" )) != EOF){
        switch(c){
            case 's':
                enableSSL = true;
                break;
            case '?':
            case 'v':
            default:
                usage();
                exit(0);
        }
    }

    Logging::OutputWriter *writer = new Logging::OutputWriter();

    Logging::MessagesManager::GetInstance()->AddWriter(Logging::INFO, writer);
    Logging::MessagesManager::GetInstance()->AddWriter(Logging::DEBUG, writer);
    Logging::MessagesManager::GetInstance()->AddWriter(Logging::ERROR, writer);


    if(enableSSL){
        Network::InitSSL();

        Network::SSLContext *context = new Network::SSLContext();
        context->Init(Network::SSL_CONTEXT_TLS_VER_1_2);

        Network::SSLContextStorage::GetInstance()->Add(ClientContext, context);
    }

    QApplication app(argc, argv);

    OptionsDialog options(enableSSL, ClientContext);
    options.setModal(true);
    options.exec();
    if(!options.IsDataSet())
        return 0;

    std::string host = options.GetHost();
    int16_t port = options.GetPort();
    std::string login = options.GetName();

    DialogWindow window;
    ClientMessagesProcessor msgProcessor;

    msgProcessor.Init(login, &window);
    window.Init(login, &msgProcessor);

    Network::ClientConnection *conn = NULL;
    if(enableSSL){
        Network::SSLClientConnection *sslConn = new Network::SSLClientConnection();
        conn = sslConn;
        sslConn->Connect(host, port, ClientContext);
    }else{
        Network::SimpleTcpClientConnection *tcpConnection = new Network::SimpleTcpClientConnection();
        tcpConnection->SetNonblockingState(false);
        conn = tcpConnection;
        tcpConnection->Connect(host, port);
    }

    Network::Client client;
    client.Init(10);
    Network::ClientConnectionId connId = client.AddConnection(conn, &msgProcessor);

    window.show();
    window.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            window.size(),
            app.desktop()->availableGeometry()
        )
    );

    std::thread networkThread(
        [&client]()
        {
            client.Run();
        }
    );

    app.exec();

    client.Stop(connId);

    networkThread.join();

    if(enableSSL)
        Network::SSLContextStorage::ReleaseInstance();

    Logging::MessagesManager::ReleaseInstance();
    return 0;
}
