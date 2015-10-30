#pragma once

#include <Connection.h>
#include <openssl/ssl.h>
#include <string>
#include <map>

namespace Network
{

void InitSSL();

DECLARE_EXCEPTION(SSLException);

enum SSLDataType
{
    SSL_DATA_TYPE_PEM,
    SSL_DATA_TYPE_ASN1,
};

enum SSLContextType
{
    SSL_CONTEXT_SSL_VER_3,
    SSL_CONTEXT_TLS_VER_1,
    SSL_CONTEXT_TLS_VER_1_1,
    SSL_CONTEXT_TLS_VER_1_2
};

class PKCS12Archive;
class X509Certificate
{
friend class PKCS12Archive;
private:
    X509 *cert;
    std::string subject;
    std::string issuer;
    int32_t since, till;
    typedef std::map<std::string, std::string> ExtensionsMap;
    ExtensionsMap extensions;
    void Init() throw (Exception);
    X509Certificate(const X509Certificate&);
    X509Certificate &operator =(const X509Certificate&) const;
public:
    typedef ExtensionsMap::const_iterator ConstIterator;
    ConstIterator BeginExtensions() const {return extensions.begin();}
    ConstIterator EndExtensions() const {return extensions.end();}
    const std::string &GetSubject() const { return subject; }
    const std::string &GetIssuer() const { return issuer;}
    int32_t GetSince() const {return since;}
    int32_t GetTill() const {return till;}
    operator X509 *() const { return cert; }
    X509Certificate():cert(NULL){}
    virtual ~X509Certificate();
    void LoadFromMemory(const std::vector<char> &Data, SSLDataType Type) throw (Exception);
    void LoadFromFile(const std::string &FilePath, SSLDataType Type) throw (Exception);
    bool GetExtension(std::string &Value) const;
};

class PrivateKey
{
friend class PKCS12Archive;
private:
    EVP_PKEY* pkey;
    PrivateKey(const PrivateKey&);
    PrivateKey &operator =(const PrivateKey&) const;
public:
    PrivateKey() : pkey(NULL){}
    virtual ~PrivateKey();
    operator EVP_PKEY *() const { return pkey; }
    void LoadFromMemory(const std::vector<char> &Data, SSLDataType Type, const std::string &PassPhrase = "") throw (Exception);
    void LoadFromFile(const std::string &FilePath, SSLDataType Type, const std::string &PassPhrase = "") throw (Exception);
};

class PKCS12Archive
{
private:
    PKCS12Archive();
    PKCS12Archive(const PKCS12Archive&);
public:
    static void Load(const std::string &FilePath, const std::string &Password, X509Certificate &Certificate, PrivateKey &PrivateKey) throw (Exception);
    static void Load(const std::vector<char> &Data, const std::string &Password, X509Certificate &Certificate, PrivateKey &PrivateKey) throw (Exception);
};

class SSLConnection : public Connection
{
private:
    mutable SSL* secureSocket;
public:
    virtual ~SSLConnection(){}
    virtual void Init(SSL *SecureSocket);
    virtual void Close() const;
    virtual void Write(const Packet &DataPacket) const throw (Exception);
    virtual Packet Read() const throw (Exception);
};

class SSLContext
{
private:
    SSL_CTX *ctx;
    SSLContext(const SSLContext&);
    SSLContext &operator= (const SSLContext&) const;
public:
    virtual ~SSLContext();
    SSLContext() : ctx(NULL){}
    void Init(SSLContextType Type, const std::string &ChiperList = "") throw (Exception);
    void UsePrivateKey(const PrivateKey &Pkey, const X509Certificate &Certificate) throw (Exception);
    void SetChiperList(const std::string &ChiperList) throw (Exception);
    operator SSL_CTX *() const
    {
        return ctx;
    }
};

typedef int8_t SSLContextKey;

class SSLContextStorage
{
private:
    typedef std::map<SSLContextKey, SSLContext*> ContextStorage;
    ContextStorage contexts;
    SSLContextStorage(){}
    SSLContextStorage(const SSLContextStorage&);
    SSLContextStorage &operator =(const SSLContextStorage&) const;
    static SSLContextStorage *instance;
public:
    virtual ~SSLContextStorage();
    static SSLContextStorage * GetInstance()
    {
        if(!instance)
            instance = new SSLContextStorage();
        return instance;
    }
    static void ReleaseInstance()
    {
        delete instance;
        instance = NULL;
    }
    void Add(SSLContextKey Key,  SSLContext* Context);
    const SSLContext* operator [] (SSLContextKey Key) const throw (Exception);
};

class SSLClientConnection : public ClientConnection
{
public:
    SSLClientConnection(){}
    virtual ~SSLClientConnection(){}
    virtual void Connect(const std::string &Host, int16_t Port, SSLContextKey ContextKey) throw (Exception);
};

class SSLServerConnection : public ServerConnection
{
private:
    const SSLContext *context;
public:
    SSLServerConnection() : context(NULL){}
    virtual ~SSLServerConnection(){}
    void Init(int16_t Port, int32_t QueueLen, SSLContextKey ContextKey) throw (Exception);
    virtual Connection *Accept() const throw (Exception);
};

};
