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

BitcoinRPC::BitcoinRPC()
{
    QPointer<QNetworkSession> session;

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
    session = new QNetworkSession(cfg);
    session->open();
    // Waits for session to be open and continues after that
    session->waitForOpened();

    // Show interface name to the user
    QNetworkInterface iff = session->interface();
    OTLog::vOutput(0, "Network opened %s\n", iff.humanReadableName().toStdString().c_str());

    QNetworkAccessManager* nam = new QNetworkAccessManager();
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
    QObject::connect(nam, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));

    QByteArray jsonString = "{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getinfo\", \"params\": [] }";
    QByteArray postDataSize = QByteArray::number(jsonString.size());

    QUrl url("http://127.0.0.1:8332");      // default is 8332

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Moneychanger");
    request.setRawHeader("X-Custom-User-Agent", "Moneychanger");
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Content-Length", postDataSize);
    QString authString = "Basic " + QString("moneychanger:money1234").toLocal8Bit().toBase64();
    //request.setRawHeader("Authorization", authString.toLocal8Bit());
    QNetworkReply* reply = nam->post(request, jsonString);
}

void BitcoinRPC::authenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    OTLog::vOutput(0, "%s\n", reply->readAll().data_ptr()->data());
    authenticator->setUser("moneychanger");
    authenticator->setPassword("money1234");
}

void BitcoinRPC::finishedSlot(QNetworkReply *reply)
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
        QByteArray bytes = reply->readAll();
        QString string(bytes);
        OTLog::vOutput(0, "%s\n", string.toStdString().c_str());
    }
    else
    {
        OTLog::vOutput(0, "%s\n", reply->errorString().toStdString().c_str());
        OTLog::vOutput(0, "%s\n", reply->readAll().data());
        switch(reply->error())
        {
        // network layer errors [relating to the destination server] (1-99):
        case QNetworkReply::ConnectionRefusedError:
            break;
        case QNetworkReply::RemoteHostClosedError:
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


