#ifndef CALLOBJECT_H
#define CALLOBJECT_H

#include <QObject>
#include <QUdpSocket>
#include <QMap>

class CallObject : public QObject
{
    Q_OBJECT
public:
    explicit CallObject(QObject *parent = nullptr);
    bool sendData(QByteArray& arr);
    bool addPeer(QString peer, short port);
    bool remPeer(QString peer, short port);
    void setHost(QString host);
    QString getHost();
    void setPort(short port);
    short getPort();
    QMap<QString,short> getPeers();
    QUdpSocket* getSocket();
    QUdpSocket* createSocket();

signals:

private:
    QUdpSocket* socket;
    QString host = "";
    short port = 0;
    QMap<QString,short> peers;
};

#endif // CALLOBJECT_H
