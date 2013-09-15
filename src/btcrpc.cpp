#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QtNetwork>
#include <QPointer>
#include <QUrl>
#include <OTLog.h>
#include "btcrpc.h"
#include "utils.h"

BtcRpc::BtcRpc()
{
    // opens network interface (step is not necessary on my computer but it was in the tutorial or something so I guess it has uses...
    InitSession();

    // set up the http header and stuff
    InitBitcoinRpc();

    // send a getinfo query over the network to see if bitcoin-qt responds
    // ConnectBitcoinRpc();
}

BtcRpc::~BtcRpc()
{
    int a = 0;
    if(this->session.isNull())
    {
        return;
    }

    this->session = NULL;   // I don't know why but it seems the object gets destroyed without my knowledge. Maybe connect the closed() signal or something...
}

void BtcRpc::InitSession()
{
    // Set Internet Access Point
    QNetworkConfigurationManager manager;

    const bool canStartIAP = (manager.capabilities() & QNetworkConfigurationManager::CanStartAndStopInterfaces);

    // Is there default access point, use it
    QNetworkConfiguration cfg = manager.defaultConfiguration();
    if(!cfg.isValid() || !canStartIAP)
    {
        //Available Access Points not found
        OTLog::Output(0, "Available Access Point not found");
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

void BtcRpc::InitBitcoinRpc()
{
    // create new network access manager
    this->rpcNAM.reset(new QNetworkAccessManager());
    // connect the function to receive replies from bitcoin qt
    QObject::connect(this->rpcNAM.data(), SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
    // connect the function to authenticate when required
    QObject::connect(this->rpcNAM.data(), SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
}

void BtcRpc::SetHeaderInformation()
{
    // without some of this, bitcoin won't understand us
    this->rpcRequest->setRawHeader("User-Agent", "Moneychanger");
    this->rpcRequest->setRawHeader("X-Custom-User-Agent", "Moneychanger");
    this->rpcRequest->setRawHeader("Content-Type", "application/json");
    //this->rpcRequest->setRawHeader("Content-Length", 0);  // varies, so we set it everytime before sending anything.
    // to authenticate instantly via header:
    //request.setRawHeader("Authorization", QString("Basic " + QString("moneychanger:money1234").toLocal8Bit().toBase64()).toLocal8Bit());  // authenticate instantly through first header:

}

bool BtcRpc::IsConnected()
{
    return this->connected;
}

bool BtcRpc::ConnectToBitcoin(QString user, QString password, QString url /*= http://127.0.0.1*/, int port /*= 8332*/)
{
    if(user == NULL || password == NULL)
        return false;
    this->username = user;
    this->password = password;

    QUrl qurl(url + ":" + QString::number(port));
    this->rpcRequest.reset(new QNetworkRequest(qurl));
    SetHeaderInformation();

    QByteArray jsonString = "{\"jsonrpc\": \"1.0\", \"id\":\"connecttest\", \"method\": \"getinfo\", \"params\": [] }";
    OTLog::Output(0, "Connecting to bitcoin, sending \"getinfo\" to test connectivity...\n");
    this->connected = true;     // needs to be set to true or else SendRpc won't send. bad design, I know..
    SendRpc(jsonString);

    if(this->connected)
        OTLog::Output(0, "Connected and authenticated successfully.\n");
    else
        OTLog::Output(0, "Couldn't connect or authenticate to bitcoin.\n");

    return true;
}

QSharedPointer<QByteArray> BtcRpc::SendRpc(const QString jsonString)
{
    return this->SendRpc(jsonString.toLocal8Bit());
}

QSharedPointer<QByteArray> BtcRpc::SendRpc(const QByteArray jsonString)
{
    if(!this->connected)
        return QSharedPointer<QByteArray>();

    QByteArray postDataSize = QByteArray::number(jsonString.size());
    this->rpcRequest->setRawHeader("Content-Length", postDataSize);

    QEventLoop loop;    // prepare loop
    connect(this->rpcNAM.data(), SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));

    QPointer<QNetworkReply> reply = this->rpcNAM->post(*this->rpcRequest, jsonString);  // send data

    // stay in loop while updating GUI and everything until bitcoin replied. or something like that.
    loop.exec();    // needs some sort of timeout. I'm sure qt offers a timer that can be linked to the quit() slot :)

    return this->rpcReplyContent;
}

void BtcRpc::ProcessReply(QSharedPointer<QByteArray> replyContType, const QSharedPointer<QByteArray> replyContent)
{
    this->rpcReplyContent = replyContent;
    return;
}

void BtcRpc::authenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    OTLog::vOutput(0, "%s\n", QString(reply->readAll()).toStdString().c_str());

    OTLog::vOutput(0, "Authenticating as %s:%s\n", this->username.toStdString().c_str(), this->password.toStdString().c_str());
    authenticator->setUser(this->username);
    authenticator->setPassword(this->password);
}

void BtcRpc::finishedSlot(QNetworkReply *reply)
{
    if(reply == NULL)
        return;

    // Reading attributes of the reply
    // e.g. the HTTP status code
    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    // Or the target URL if it was a redirect:
    QVariant redirectionTargetUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    // see CS001432 on how to handle this


    // error received?
    if(!reply->error() == QNetworkReply::NoError)
    {
        ProcessErrorMessage(reply);        
        // whichever error occurs, we can assume that we aren't connected to bitcoin:
        this->connected = false;
    }
    else
    {
        this->connected = true;
    }

    QSharedPointer<QByteArray> replyContType(new QByteArray(reply->rawHeader("Content-Type")));
    QSharedPointer<QByteArray> replyContent(new QByteArray(reply->readAll()));          // I think readAll() only works once, so here's where it is used.
    //OTLog::vOutput(0, "%s\n", QString(*replyContent).toStdString().c_str());  // debug output, not needed anymore.
    //OTLog::vOutput(0, "Content-Type: %s\n", QString(reply->rawHeader("Content-Type")).toStdString().c_str());
    ProcessReply(replyContType, replyContent);

    // We receive ownership of the reply object
    // and therefore need to handle deletion
    delete reply;
}

void BtcRpc::ProcessErrorMessage(const QNetworkReply* reply)
{
    OTLog::vOutput(0, "Error connecting to bitcoin: %s\n", reply->errorString().toStdString().c_str());
    //OTLog::vOutput(0, "%s\n", QString(reply->readAll()).toStdString().c_str());

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


