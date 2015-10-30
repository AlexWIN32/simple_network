#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/bio.h>
#include <SSLConnection.h>
#include <PacketProcessor.h>
#include <SimpleTcpConnection.h>
#include <SocketLevel.h>
#include <cstdio>

namespace Network
{

SSLContextStorage *SSLContextStorage::instance = NULL;

static std::string GetSSLErrorString()
{
    const size_t buffLen = 120;
    char buff[buffLen];

    unsigned long errCode = ERR_get_error();
    ERR_error_string_n(errCode, buff, buffLen);

    return buff;
}

void InitSSL()
{
    SSL_library_init();
    OpenSSL_add_all_digests();
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
}

X509Certificate::~X509Certificate()
{
    X509_free(cert);
}

static X509 *GetCertificateFromBIO(BIO *In, SSLDataType Type) throw (Exception)
{
    X509 *cert;

    if(Type == SSL_DATA_TYPE_ASN1)
        cert = d2i_X509_bio(In, NULL);
    else
        cert = PEM_read_bio_X509(In, NULL, NULL, NULL);

    if(cert == NULL)
        throw SSLException("cant load private certificate: " + GetSSLErrorString());

    return cert;
}

inline unsigned char GetDigth(const unsigned char *Str)
{
    return (Str[0] - '0') * 10 + Str[1] - '0';
}

static time_t ConvertASNTimeToTimestamp(const ASN1_UTCTIME *s)
{
    tm data;
    memset(&data, 0, sizeof(data));

    data.tm_year = GetDigth(s->data);
    if(data.tm_year < 50)
        data.tm_year += 100;

    data.tm_mon  = GetDigth(s->data + 2) - 1;
    data.tm_mday = GetDigth(s->data + 4);
    data.tm_hour = GetDigth(s->data + 6);
    data.tm_min  = GetDigth(s->data + 8);
    data.tm_sec  = GetDigth(s->data + 10);

    int offset = 0;
    if(s->data[12] == 'Z')
        offset = 0;
    else
    {
        offset = GetDigth(s->data + 13) * 60 + GetDigth(s->data + 15);
        if(s->data[12] == '-')
            offset = -offset;
    }

    return mktime(&data) - offset * 60;
}

void X509Certificate::Init() throw (Exception)
{
    char *data = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
    subject = data;
    OPENSSL_free(data);

    data = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
    issuer = data;
    OPENSSL_free(data);

    int extCount = X509_get_ext_count(cert);
    if(extCount >= 0) {

        for(int e = 0; e < extCount; e++){

            X509_EXTENSION *ext = X509_get_ext(cert, e);

            const size_t BuffLen = 80;
            char buff[BuffLen];
            OBJ_obj2txt(buff, BuffLen, X509_EXTENSION_get_object(ext), true);

            std::string key, data;
            key.assign(buff, BuffLen);
            data.assign(reinterpret_cast<char*>(ext->value->data), ext->value->length);

            extensions.insert(std::make_pair(key, data));
        }
    }

    since = ConvertASNTimeToTimestamp(X509_get_notBefore(cert));
    till = ConvertASNTimeToTimestamp(X509_get_notAfter(cert));
}

void X509Certificate::LoadFromMemory(const std::vector<char> &data, SSLDataType Type) throw (Exception)
{
    BIO *in = BIO_new(BIO_s_mem());
    if(!in)
        throw SSLException("cant allocate BIO memory");

    if(BIO_write(in, &data[0], data.size()) <= 0) //AAA vector addr
        throw SSLException("cant write to BIO memory");

    try{
        cert = GetCertificateFromBIO(in, Type);
    }catch(const Exception &ex){
        BIO_free(in);
        throw ex;
    }

    Init();

    BIO_free(in);
}

void X509Certificate::LoadFromFile(const std::string &FilePath, SSLDataType Type) throw (Exception)
{
    BIO *in = BIO_new(BIO_s_file());
    if(!in)
        throw SSLException("cant allocate BIO memory");

    if(BIO_read_filename(in, FilePath.c_str()) <= 0)
        throw SSLException("cant read data from file " + FilePath +": " + GetSSLErrorString());

    try{
        cert = GetCertificateFromBIO(in, Type);
    }catch(const Exception &ex){
        BIO_free(in);
        throw ex;
    }

    Init();

    BIO_free(in);
}

void PKCS12Archive::Load(const std::string &FilePath, const std::string &Password, X509Certificate &Certificate, PrivateKey &PrivateKey) throw (Exception)
{
    BIO *in = BIO_new(BIO_s_file());
    if(!in)
        throw SSLException("cant allocate BIO memory");

    if(BIO_read_filename(in, FilePath.c_str()) <= 0)
        throw SSLException("cant read data from file " + FilePath +": " + GetSSLErrorString());

    PKCS12 *p12 = d2i_PKCS12_bio(in, NULL);
    if (p12 == NULL){
        BIO_free(in);
        throw SSLException("cant load PKCS12 from file " + FilePath +": " + GetSSLErrorString());
    }

    EVP_PKEY *pkey;
    X509 *cert;

    if(!PKCS12_parse(p12, Password.c_str(), &pkey, &cert, NULL)){
        BIO_free(in);
        throw SSLException("cant parse PKCS12 from file " + FilePath + ": " + GetSSLErrorString());
    }

    Certificate.cert = cert;
    Certificate.Init();

    PrivateKey.pkey = pkey;

    PKCS12_free(p12);
    BIO_free(in);
}

bool X509Certificate::GetExtension(std::string &Value) const
{
    ExtensionsMap::const_iterator ci = extensions.find(Value);
    if(ci != extensions.end())
        return false;

    Value = ci->second;
    return true;
}

static int passwd_callback(char *buf, int size, int, void *password)
{
    strncpy(buf, reinterpret_cast<char*>(password), size);
    buf[size - 1] = '\0';
    return strlen(buf);
}

static EVP_PKEY *GetKeyFromBIO(BIO *In, SSLDataType Type, const std::string &PassPhrase) throw (Exception)
{
    EVP_PKEY *key;

    char *userData = NULL;

    if(Type == SSL_DATA_TYPE_ASN1)
        key = d2i_PrivateKey_bio(In, NULL);
    else{
        if(PassPhrase != ""){
            userData = new char[PassPhrase.size() + 1];
            memcpy(userData, PassPhrase.c_str(), PassPhrase.size());
            userData[PassPhrase.size()] = '\0';

            key = PEM_read_bio_PrivateKey(In, NULL, passwd_callback, userData);
        }else
            key = PEM_read_bio_PrivateKey(In, NULL, NULL, NULL);
    }

    delete [] userData;

    if(key == NULL)
        throw SSLException("cant load private key: " + GetSSLErrorString());

    return key;
}

void PrivateKey::LoadFromMemory(const std::vector<char> &data, SSLDataType Type, const std::string &PassPhrase) throw (Exception)
{
    BIO *in = BIO_new(BIO_s_mem());
    if(!in)
        throw SSLException("cant allocate BIO memory");

    if(BIO_write(in, &data[0], data.size()) <= 0) //AAA vector addr
        throw SSLException("cant write to BIO memory");

    try{
        pkey = GetKeyFromBIO(in, Type, PassPhrase);
    }catch(const Exception &ex){
        BIO_free(in);
        throw ex;
    }

    BIO_free(in);
}

void PrivateKey::LoadFromFile(const std::string &FilePath, SSLDataType Type, const std::string &PassPhrase) throw (Exception)
{
    BIO *in = BIO_new(BIO_s_file());
    if(!in)
        throw SSLException("cant allocate BIO memory");

    if(BIO_read_filename(in, FilePath.c_str()) <= 0)
        throw SSLException("cant read data from file " + FilePath +": " + GetSSLErrorString());

    try{
        pkey = GetKeyFromBIO(in, Type, PassPhrase);
    }catch(const Exception &ex){
        BIO_free(in);
        throw ex;
    }

    BIO_free(in);
}

PrivateKey::~PrivateKey()
{
    EVP_PKEY_free(pkey);
}

inline int32_t WriteToSSLConnection(SSL* SecureSocket, const void *Data, size_t DataLen) throw (Exception)
{
    int32_t ret = SSL_write(SecureSocket, Data, DataLen);
    if(ret <= 0)
        throw SSLException("write error:" + GetSSLErrorString());

    return ret;
}

inline int32_t ReadFromSSLConnection(SSL* SecureSocket, void *Data, size_t DataLen) throw (Exception)
{
    int32_t cnt = SSL_read(SecureSocket, Data, DataLen);
    if(cnt == 0 && SSL_get_error(SecureSocket, cnt) == SSL_ERROR_ZERO_RETURN)
        return 0;

    if(cnt < 0)
         throw SSLException("read error:" + GetSSLErrorString());

    return cnt;
}

void SSLConnection::Init(SSL* SecureSocket)
{
    secureSocket = SecureSocket;
    socket = SSL_get_fd(secureSocket);
    isClosed = false;
}

void SSLConnection::Close() const
{
    SSL_shutdown(secureSocket);
    SocketLevel::close_socket(socket);
    SSL_free(secureSocket);
    isClosed = true;
}

void SSLConnection::Write(const Packet &DataPacket) const throw (Exception)
{
    if(isClosed)
        return;

    PacketProcessor::Write<SSL*>(secureSocket, DataPacket, WriteToSSLConnection);
}

Packet SSLConnection::Read() const throw (Exception)
{
    if(isClosed)
        return Packet();

    return PacketProcessor::Read<SSL*>(isClosed, secureSocket, ReadFromSSLConnection);
}

SSLContext::~SSLContext()
{
    SSL_CTX_free(ctx);
}

void SSLContext::Init(SSLContextType Type, const std::string &ChiperList) throw (Exception)
{
    const SSL_METHOD *method;

    if(Type == SSL_CONTEXT_SSL_VER_3)
        method = SSLv3_method();
    else if(Type == SSL_CONTEXT_TLS_VER_1)
        method = TLSv1_method();
    else if(Type == SSL_CONTEXT_TLS_VER_1_1)
        method = TLSv1_1_method();
    else if(Type == SSL_CONTEXT_TLS_VER_1_2)
        method = TLSv1_2_method();
    else
        throw SSLException("Unknown SSL method");

    ctx = SSL_CTX_new(method);
    if(!ctx)
        throw SSLException("cant create SSL context:" + GetSSLErrorString());

    if(ChiperList != "")
        SetChiperList(ChiperList);
}

void SSLContext::UsePrivateKey(const PrivateKey &Pkey, const X509Certificate &Certificate) throw (Exception)
{
    if(SSL_CTX_use_certificate(ctx, Certificate) <= 0)
        throw SSLException("cant set certificate to context:" + GetSSLErrorString());

    if(SSL_CTX_use_PrivateKey(ctx, Pkey) <= 0)
        throw SSLException("cant set private key to context:" + GetSSLErrorString());

    if(!SSL_CTX_check_private_key(ctx))
        throw SSLException("Private key is not conform to the certificate:" + GetSSLErrorString());
}

void SSLContext::SetChiperList(const std::string &ChiperList) throw (Exception)
{
    int res = SSL_CTX_set_cipher_list(ctx, ChiperList.c_str());
    if(res == 0)
        throw SSLException("cant set SSL chiper list:" + GetSSLErrorString());
}

SSLContextStorage::~SSLContextStorage()
{
    ContextStorage::const_iterator ci;
    for(ci = contexts.begin(); ci != contexts.end(); ++ci)
        delete ci->second;
}

void SSLContextStorage::Add(SSLContextKey Key,  SSLContext* Context)
{
    contexts.insert(std::make_pair(Key, Context));
}

const SSLContext* SSLContextStorage::operator [] (SSLContextKey Key) const throw (Exception)
{
    ContextStorage::const_iterator ci = contexts.find(Key);
    if(ci == contexts.end())
        throw SSLException("SSL context not found");

    return ci->second;
}

void SSLClientConnection::Connect(const std::string &Host, int16_t Port, SSLContextKey ContextKey) throw (Exception)
{
    const SSLContext *context = (*SSLContextStorage::GetInstance())[ContextKey];

    SocketHandle socket = SocketLevel::create_tcp_client_socket(Host, Port);

    SSL *ssl = SSL_new(*context);
    SSL_set_fd(ssl, socket);

    if(SSL_connect(ssl) <= 0)
        throw SSLException("cant connect:" + GetSSLErrorString());

    SSLConnection *sslConnection = new SSLConnection();
    sslConnection->Init(ssl);
    handle = sslConnection;
}

void SSLServerConnection::Init(int16_t Port, int32_t QueueLen, SSLContextKey ContextKey) throw (Exception)
{
    context = (*SSLContextStorage::GetInstance())[ContextKey];

    SimpleTcpConnection *listenConn = new SimpleTcpConnection();
    listenConn->Init(SocketLevel::create_tcp_server_socket(Port, QueueLen));
    handle = listenConn;
}

Connection *SSLServerConnection::Accept() const throw (Exception)
{
    SocketHandle sock = SocketLevel::accept_from_tcp_server_socket(handle->GetHandle());

    SSL *ssl = SSL_new(*context);
    SSL_set_fd(ssl, sock);

    if(SSL_accept(ssl) <= 0)
        throw SSLException("cant accept:" + GetSSLErrorString());

    SSLConnection *sslConnection = new SSLConnection();
    sslConnection->Init(ssl);
    return sslConnection;
}

};
