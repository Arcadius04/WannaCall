#include "callobject.h"

CallObject::CallObject(QObject *parent)
    : QObject{parent}
{
}

bool CallObject::sendData(QByteArray& arr){
    for(QString peerAddress : peers.keys()){
        long long ret = socket->writeDatagram(arr,QHostAddress(peerAddress),peers.value(peerAddress));
        qDebug() << ret << " sending to " << peerAddress << ":" << peers.value(peerAddress) << " " << arr.size() << " bytes.";
        if(ret == -1){
            qDebug() << "Failed to send datagram:" << socket->errorString();
        }
    }
    return true;
}

bool CallObject::addPeer(QString peer, short port){
    peers.insert(peer, port);
    return true;
}

bool CallObject::remPeer(QString peer, short port){
    peers.remove(peer);
    return true;
}

void CallObject::setHost(QString host){
    this->host = host;
}
QString CallObject::getHost(){
    return this->host;
}
void CallObject::setPort(short port){
    this->port = port;
}
short CallObject::getPort(){
    return this->port;
}
QMap<QString,short> CallObject::getPeers(){
    return this->peers;
}
QUdpSocket* CallObject::getSocket(){
    return this->socket;
}
QUdpSocket* CallObject::createSocket(){
    this->socket = new QUdpSocket(this);

    if(!socket->bind(QHostAddress(host),port)){
        qDebug() << "Failed to bind to " << host << ":" << port;
    }else{
        qDebug() << "Successfully binded to " << host << ":" << port;
    }
    if(!socket->open(QIODevice::ReadWrite)){
        qDebug() << "Failed to open socket on read and write mode!";
    }else{
        qDebug() << "Successfully opened socket.";
    }
    return this->socket;
}
