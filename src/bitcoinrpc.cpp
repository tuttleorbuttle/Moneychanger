#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkReply>>
#include <QNetworkAccessManager>
#include <QtNetwork>
#include <QPointer>
#include <QUrl>
#include <OTLog.h>
#include "bitcoinrpc.h"

BitcoinRpc::BitcoinRpc()
{
    // opens network interface (step is not necessary on my computer but it was in the tutorial or something so I guess it has uses...
    InitSession();
    // set up the http header and stuff
    InitBitcoinRpc();
    // send a getinfo query over the network to see if bitcoin-qt responds
    ConnectBitcoinRpc();
}

BitcoinRpc::~BitcoinRpc()
{
    int a = 0;
    if(this->session.isNull())
    {
        return;
    }

    this->session = NULL;   // I don't know why but it seems the object gets destroyed without my knowledge. Maybe connect the closed() signal or something...
}

void BitcoinRpc::InitSession()
{
    // Set Internet Access Point
    QNetworkConfigurationManager manager;

    const bool canStartIAP = (manager.capabilities() & QNetworkConfigurationManager::CanStartAndStopInterfaces);

    // Is there default access point, use it
    QNetworkConfiguration cfg = manager.defaultConfiguration();
    if(!cfg.isValid() || !canStartIAP)
    {
        //Available Access Points not found
        OTLog::Output(0, "Acailable Access Point not found");
        return;
    }

    // Open session
    this->session = new QNetworkSession(cfg);
    this->session->open();
    // Waits for session to be open and continues after that
    this->session->waitForOpened();

    // Show interface name to the user
    QNetworkInterface iff = this->session->interface();
    OTLog::vOutput(0, "Network opened on %s\n", iff.humanReadableName().toStdString().c_str());
}

void BitcoinRpc::InitBitcoinRpc()
{
    this->rpcNAM.reset(new QNetworkAccessManager());
    QObject::connect(&(*rpcNAM), SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
    QObject::connect(&(*rpcNAM), SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));

    QUrl url("http://127.0.0.1:8332");      // default is 8332
    this->rpcRequest.reset(new QNetworkRequest(url));
    this->rpcRequest->setRawHeader("User-Agent", "Moneychanger");
    this->rpcRequest->setRawHeader("X-Custom-User-Agent", "Moneychanger");
    this->rpcRequest->setRawHeader("Content-Type", "application/json");
    //this->rpcRequest->setRawHeader("Content-Length", 0);
    //request.setRawHeader("Authorization", QString("Basic " + QString("moneychanger:money1234").toLocal8Bit().toBase64()).toLocal8Bit());  // authenticate instantly through first header:
}

void BitcoinRpc::ConnectBitcoinRpc()
{
    QByteArray jsonString = "{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getinfo\", \"params\": [] }";
    OTLog::Output(0, "Connecting to bitcoin on port 8332, sending \"getinfo\"...");
    SendRpc(jsonString);
}

QString BitcoinRpc::SendRpc(QString jsonString)
{
    QByteArray postDataSize = QByteArray::number(jsonString.size());
    this->rpcRequest->setRawHeader("Content-Length", postDataSize);
    QPointer<QNetworkReply> reply = this->rpcNAM->post(*this->rpcRequest, jsonString.toLocal8Bit());
    OTLog::vOutput(0, "Reply: %s", QString(reply->readAll()).toStdString().c_str());
    return QString(reply->readAll());
}
QString BitcoinRpc::SendRpc(QByteArray jsonString)
{
    QByteArray postDataSize = QByteArray::number(jsonString.size());
    this->rpcRequest->setRawHeader("Content-Length", postDataSize);
    QPointer<QNetworkReply> reply = this->rpcNAM->post(*this->rpcRequest, jsonString);
    OTLog::vOutput(0, "Reply: %s", QString(reply->readAll()).toStdString().c_str());
    return QString(reply->readAll());
}

QString BitcoinRpc::SendRpc(QString jsonString, QString& response)
{
    // not yet implemented, maybe never...
}

void BitcoinRpc::ProcessReply(QSharedPointer<QByteArray> replyContType, QSharedPointer<QByteArray> replyContent)
{
    if(this->StringProcessors[*replyContType] != NULL)
    {
        (*this->StringProcessors[*replyContType])(replyContent);
        int a = 0;
    }
}

void BitcoinRpc::RegisterStringProcessor(QByteArray contentType, ProcessString delegate)
{
    if(contentType.size() == 0)
        return;

    this->StringProcessors[contentType] = delegate;
}

void BitcoinRpc::authenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    OTLog::vOutput(0, "%s\n", QString(reply->readAll()).toStdString().c_str());

    QString user("moneychanger");
    QString pass("money1234");
    OTLog::vOutput(0, "Authenticating as %s:%s\n", user.toStdString().c_str(), pass.toStdString().c_str());
    authenticator->setUser(user);
    authenticator->setPassword(pass);
}

void BitcoinRpc::finishedSlot(QNetworkReply *reply)
{
    if(reply == NULL)
        return;

    // Reading attributes of the reply
    // e.g. the HTTP status code
    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    // Or the target URL if it was a redirect:
    QVariant redirectionTargetUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    // see CS001432 on how to handle this

    // no error received?
    if(reply->error() == QNetworkReply::NoError)
    {
        // read data from QNetworkReply here

        // Example 1: Creating QImage from the replay
        //QImageReader imageReader(reply);
        //QImage pic = imageReader.read();

        // Example 2: Reading bytes from the reply
        QSharedPointer<QByteArray> replyContType(new QByteArray(reply->rawHeader("Content-Type")));
        QSharedPointer<QByteArray> replyContent(new QByteArray(reply->readAll()));
        OTLog::vOutput(0, "%s\n", QString(*replyContent).toStdString().c_str());
        OTLog::vOutput(0, "Content-Type: %s\n", QString(reply->rawHeader("Content-Type")).toStdString().c_str());
        ProcessReply(replyContType, replyContent);
    }
    else
    {
        OTLog::vOutput(0, "Error connecting to bitcoin: %s\n", reply->errorString().toStdString().c_str());
        OTLog::vOutput(0, "%s\n", QString(reply->readAll()).toStdString().c_str());
        switch(reply->error())
        {
        // network layer errors [relating to the destination server] (1-99):
        case QNetworkReply::ConnectionRefusedError:
            break;
        case QNetworkReply::RemoteHostClosedError:
            OTLog::Output(0, "Connection was closed. This also occurs when bitcoin-qt is not running or not accpting a connection on this port.");
            break;
        case QNetworkReply::HostNotFoundError:
            break;
        case QNetworkReply::TimeoutError:
            break;
        case QNetworkReply::OperationCanceledError:
            break;
        case QNetworkReply::SslHandshakeFailedError:
            break;
        case QNetworkReply::TemporaryNetworkFailureError:
            break;
        case QNetworkReply::NetworkSessionFailedError:
            break;
        case QNetworkReply::BackgroundRequestNotAllowedError:
            break;
        case QNetworkReply::UnknownNetworkError:
            break;

        // proxy errors (101-199):
        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
        case QNetworkReply::ProxyAuthenticationRequiredError:
        case QNetworkReply::UnknownProxyError:
            break;

        // content errors (201-299):
        case QNetworkReply::ContentAccessDenied:
        case QNetworkReply::ContentOperationNotPermittedError:
        case QNetworkReply::ContentNotFoundError:
        case QNetworkReply::AuthenticationRequiredError:
        case QNetworkReply::ContentReSendError:
        case QNetworkReply::UnknownContentError:
            break;

        // protocol errors
        case QNetworkReply::ProtocolUnknownError:
        case QNetworkReply::ProtocolInvalidOperationError:
        case QNetworkReply::ProtocolFailure:
        default:
            break;
        }

        // some http error received
    }

    // We receive ownership of the reply object
    // and therefore need to handle deletion
    delete reply;
}


