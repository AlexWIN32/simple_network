#pragma once
#include <QWidget>
#include <QDialog>
#include <ViewController.h>
#include <SSLConnection.h>

class QLineEdit;
class QTextEdit;
class QPushButton;

class OptionsDialog : public QDialog
{
    Q_OBJECT

private:
    QLineEdit *nameLine, *serverLine, *portLine;
    QPushButton *okButton, *cancelButton;
    bool dataSet;
    bool useSSL;
    Network::SSLContextKey contextKey;
private Q_SLOTS:
    void apply();
public:
    OptionsDialog(bool UseSSL, Network::SSLContextKey ContextKey);
    bool IsDataSet() const {return dataSet;}
    std::string GetName() const;
    std::string GetHost() const;
    int16_t GetPort() const;
};

class DialogWindow : public QWidget, public ViewController
{
    Q_OBJECT

public:
    DialogWindow(QWidget *parent = 0);
    void Init(const std::string &Login, ClientMessagesProcessor *MsgProcessor)
    {
        msgProcessor = MsgProcessor;
        login = Login;
    }
private Q_SLOTS:
    void SendMessage();
    void ProcessLostConnection();
private:
    QLineEdit *nameLine;
    QTextEdit *addressText;
    QPushButton *sendButton;
    std::string login;
    virtual void ShowMessage(const std::string &Login, const std::string &Message) throw (Exception);
    virtual void ShowUserLogin(const std::string &Login) throw (Exception);
    virtual void ShowUserLogout(const std::string &Login) throw (Exception);
    virtual void ShowLostConnection() throw (Exception);
protected:
    virtual void keyPressEvent(QKeyEvent *event);
};
