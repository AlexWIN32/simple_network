#include <QtCore>
#include <QtGui>
#include <QDialog>
#include <DialogWindow.h>
#include <SimpleTcpConnection.h>
#include <Serializing.h>
#include <Actions.h>
#include <Variables.h>
#include <ClientMessagesProcessor.h>
#include <memory>

std::string OptionsDialog::GetName() const
{
    return nameLine->text().toStdString();
}

std::string OptionsDialog::GetHost() const
{
    return serverLine->text().toStdString();
}

int16_t OptionsDialog::GetPort() const
{
    return USParser::FromString(portLine->text().toStdString());
}

void OptionsDialog::apply()
{
    std::string serverHost = serverLine->text().toStdString();
    int16_t port = USParser::FromString(portLine->text().toStdString());

    Network::ClientConnection *conn = NULL;
    try{
        if(useSSL){
            Network::SSLClientConnection *sslConn = new Network::SSLClientConnection();
            conn = sslConn;
            sslConn->Connect(serverHost, port, contextKey);
        }else{
            Network::SimpleTcpClientConnection *tcpConn = new Network::SimpleTcpClientConnection();
            conn = tcpConn;
            tcpConn->Connect(serverHost, port);
        }
    }catch(const Exception &ex){
        QMessageBox::critical(this, tr("Error"), tr("Cant connect to host: ") + ex.msg.c_str());
        delete conn;
        return;
    }

    std::auto_ptr<Network::ClientConnection> testConn(conn);

    Network::ConnectionHandler<Network::ClientConnection> handler(*testConn);

    Network::Packet nameQueryPacket(QRY_NAME_CHECK);
    nameQueryPacket.CreateVariable(VAR_USER_NAME, nameLine->text().toStdString());

    try{
        handler.Get().Send(nameQueryPacket);

        Network::Packet resp = handler.Get().Recieve();

        if(resp.GetAction() == QRY_ASK_NAME_IS_USED){
            QMessageBox::critical(this, tr("Error"), tr("Name is used"));
            return;
        }

        if(resp.GetAction() == Network::ACTION_ERROR)
        {
            QMessageBox::critical(this, tr("Error"), tr("Internal server error"));
            return;
        }

    }catch(const Exception &){
        QMessageBox::critical(this, tr("Error"), tr("Network error"));
        return;
    }

    dataSet = true;

    close();
}

OptionsDialog::OptionsDialog(bool UseSSL, Network::SSLContextKey ContextKey)
    : dataSet(false), useSSL(UseSSL), contextKey(ContextKey)
{

    nameLine = new QLineEdit();
    serverLine = new QLineEdit();
    portLine = new QLineEdit();

    QGridLayout *serverLayout = new QGridLayout;
    serverLayout->addWidget(new QLabel(tr("Server")), 0, 0);
    serverLayout->addWidget(serverLine, 0, 1);
    serverLayout->addWidget(new QLabel(tr("Port")), 0, 2);
    serverLayout->addWidget(portLine, 0, 3);

    QGridLayout *nameLayout = new QGridLayout;
    nameLayout->addWidget(new QLabel(tr("Name")), 0, 0, Qt::AlignRight);
    nameLayout->addWidget(nameLine, 0, 1);

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(apply()));

    QGridLayout *btnLayout = new QGridLayout;
    btnLayout->addWidget(okButton, 0, 0, Qt::AlignRight);
    btnLayout->addWidget(cancelButton, 0, 1, Qt::AlignLeft);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addLayout(serverLayout, 0, 0);
    mainLayout->addLayout(nameLayout, 1, 0);
    mainLayout->addLayout(btnLayout, 2, 0);

    setLayout(mainLayout);
    setWindowTitle(tr("Options"));
}

DialogWindow::DialogWindow(QWidget *parent)
    : QWidget(parent)
{
    nameLine = new QLineEdit();

    addressText = new QTextEdit();
    addressText->setReadOnly(true);

    sendButton = new QPushButton(tr("Send"));

    connect(sendButton, SIGNAL(clicked()), this, SLOT(SendMessage()));

    QGridLayout *messageLayout = new QGridLayout;
    messageLayout->addWidget(sendButton, 0, 0);
    messageLayout->addWidget(nameLine, 0, 1);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(addressText, 0, 0);
    mainLayout->addLayout(messageLayout, 1, 0);

    setLayout(mainLayout);
    setWindowTitle(tr("Simple messanger"));
}

void DialogWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()){
        case Qt::Key_Escape:
            close();
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            SendMessage();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

void DialogWindow::ShowMessage(const std::string &Login, const std::string &Message) throw (Exception)
{
    addressText->moveCursor(QTextCursor::End);
    addressText->append(std::string(Login + ":" + Message).c_str());
    addressText->moveCursor(QTextCursor::End);
}

void DialogWindow::ShowUserLogin(const std::string &Login) throw (Exception)
{
    if(login == Login)
        return;

    addressText->moveCursor(QTextCursor::End);
    addressText->append(QString(Login.c_str()) +" " + tr("join"));
    addressText->moveCursor(QTextCursor::End);
}

void DialogWindow::ShowUserLogout(const std::string &Login) throw (Exception)
{
    if(login == Login)
        return;

    addressText->moveCursor(QTextCursor::End);
    addressText->append(QString(Login.c_str()) +" " + tr("leave"));
    addressText->moveCursor(QTextCursor::End);
}

void DialogWindow::SendMessage()
{
    std::string msg = nameLine->text().toStdString();
    if(msg == "")
        return;

    msgProcessor->SendMessage(msg);
    nameLine->clear();
}

void DialogWindow::ProcessLostConnection()
{
    QMessageBox::critical(this, tr("Error"), tr("Connection lost"));
    close();
}

void DialogWindow::ShowLostConnection() throw (Exception)
{
    QMetaObject::invokeMethod( this, "ProcessLostConnection");
}
