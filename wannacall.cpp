#include "wannacall.h"
#include "./ui_wannacall.h"

WannaCall::WannaCall(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WannaCall)
{
    ui->setupUi(this);
    setWindowTitle("Wanna Call");
    setWindowIcon(QIcon(":/icons/peer-to-peer.png"));
    resize(QSize(700,250));

    ui->treeWidget->setHeaderLabels(QStringList() << "IP" << "PORT");
    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->setHeaderHidden(false);

    this->settings = new Settings(this);
    this->settings->hide();

    if(port != 0){ // If port is defined then there is no need for ports and all clients will have the same port.
        ui->portLabel->hide();
        ui->portLineEdit->hide();
        ui->peerPortLabel->hide();
        ui->peerPortLineEdit->hide();
    }

    // First step is to just request the user's IP and Port.
    ui->endButton->hide();
    ui->peerIPLabel->hide();
    ui->peerIPLineEdit->hide();
    ui->peerPortLabel->hide();
    ui->peerPortLineEdit->hide();
    ui->addPeerButton->hide();
    ui->remPeerButton->hide();
    ui->peersLabel->hide();
    ui->treeWidget->hide();

    format.setSampleRate(44100);
    format.setSampleFormat(QAudioFormat::Float);
    format.setChannelCount(2);

    qDebug() << "Ran constructor";
}

WannaCall::~WannaCall()
{
    if(inpDev != nullptr){
        inpDev->disconnect();
        inpDev = nullptr;
    }
    if(input != nullptr){
        if(input->state() == QAudio::ActiveState){
            input->stop();
        }
    }
    if(outDev != nullptr){
        outDev->disconnect();
        outDev = nullptr;
    }
    if(output != nullptr){
        if(output->state() == QAudio::ActiveState){
            output->stop();
        }
    }
    if(call != nullptr){
        call->getSocket()->close();
        qDebug() << "Successfully closed socket";
        call->getPeers().clear();
        qDebug() << "Cleared peers!";
        delete call;
        call = nullptr;
    }
    if(settings != nullptr){
        delete settings;
    }
    delete ui;
}

void WannaCall::refreshPeers(){
    ui->treeWidget->clear();
    for(QString ip : call->getPeers().keys()){
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0,ip);
        item->setText(1,QString::number(call->getPeers().value(ip)));
        ui->treeWidget->addTopLevelItem(item);
    }
}

int WannaCall::inputUpdate(const QString &text)
{
    if(call != nullptr){
        qDebug() << "Can't switch input in a call!";
        return -3;
    }
    QList<QAudioDevice> devices = QMediaDevices::audioInputs();
    for(QAudioDevice dev : devices){
        if(dev.description() == text){
            if(this->input != nullptr){
                if(this->input->state() == QAudio::ActiveState){
                    this->inpDev->disconnect();
                    this->input->stop();
                    this->inpDev = nullptr;
                }
                delete this->input;
            }
            if(dev.isFormatSupported(format)){
                this->input = new QAudioSource(dev,format);
                qDebug() << "Selected input " << dev.description();
                return 0;
            }else{
                qDebug() << "unsupported format for input.";
                return -1;
            }
        }
    }
    return -2;
}


int WannaCall::listenUpdate()
{
    if(this->input != nullptr && this->output != nullptr){
        if(this->call != nullptr){
            qDebug() << "Can't listen when call is active!";
            return -1;
        }
        if(this->input->state() == QAudio::ActiveState
            && this->output->state() == QAudio::ActiveState){
            qDebug() << "Listening is already active. Returning.";
            return -2;
        }
        this->input->setVolume(100);
        inpDev = this->input->start();

        this->output->setVolume(100);
        outDev = this->output->start();
        qDebug() << inpDev << ", " << outDev;

        if(inpDev != nullptr){
            connect(inpDev, &QIODevice::readyRead, [this](){
                if(inpDev != nullptr && outDev != nullptr){
                    QByteArray bArr = inpDev->readAll();
                    outDev->write(bArr);
                }
            });
        }else{
            return -3;
        }
        listening = true;
        return 0;
    }
    return -4;
}


int WannaCall::outputUpdate(const QString &text)
{
    if(call != nullptr){
        qDebug() << "Can't switch output in a call!";
        return -3;
    }
    qDebug() << text;
    QList<QAudioDevice> devices = QMediaDevices::audioOutputs();
    for(QAudioDevice dev : devices){
        if(dev.description() == text){
            if(this->output != nullptr){
                if(this->output->state() == QAudio::ActiveState){
                    this->outDev->disconnect();
                    this->output->stop();
                    this->outDev = nullptr;
                }
                delete this->output;
            }
            if(dev.isFormatSupported(format)){
                this->output = new QAudioSink(dev,format);
                qDebug() << "Selected output " << dev.description();
                qDebug() << "format : " << format;
                return 0;
            }else{
                qDebug() << "format not supported";
                return -1;
            }
        }
    }
    return -2;
}


void WannaCall::on_addPeerButton_clicked()
{
    if(call->getPeers().contains(ui->peerIPLineEdit->text())){
        qDebug() << "Peer already exists!";
        return;
    }
    if(port != 0){
        call->addPeer(ui->peerIPLineEdit->text(), port);
    } else {
        call->addPeer(ui->peerIPLineEdit->text(), ui->peerPortLineEdit->text().toShort());
    }
    ui->peerIPLineEdit->clear();
    ui->peerPortLineEdit->clear();
    refreshPeers();
}


void WannaCall::on_remPeerButton_clicked()
{
    if(!call->getPeers().contains(ui->peerIPLineEdit->text())){
        qDebug() << "Peer does not exists!";
        return;
    }
    if(port != 0){
        call->remPeer(ui->peerIPLineEdit->text(), port);
    } else {
        call->remPeer(ui->peerIPLineEdit->text(), ui->peerPortLineEdit->text().toShort());
    }
    ui->peerIPLineEdit->clear();
    ui->peerPortLineEdit->clear();
    refreshPeers();
}


void WannaCall::on_callButton_clicked()
{
    if(call != nullptr){
        qDebug() << "Call already established!";
        return;
    }
    if(listening){
        if(inpDev != nullptr){
            inpDev->disconnect();
        }
        if(outDev != nullptr){
            outDev->disconnect();
        }
        listening = false;
    }
    if(this->input == nullptr){
        this->input = new QAudioSource(QMediaDevices::defaultAudioInput(),format);
    }
    if(this->output == nullptr){
        this->output = new QAudioSink(QMediaDevices::defaultAudioOutput(),format);
    }
    call = new CallObject(this);
    call->setHost(ui->ipLineEdit->text());
    if(port != 0){
        call->setPort(port);
    } else {
        call->setPort(ui->portLineEdit->text().toShort());
    }
    qDebug() << "Successfully created call!";
    call->createSocket();

    this->input->setVolume(100);
    inpDev = this->input->start();

    this->output->setVolume(100);
    outDev = this->output->start();

    connect(inpDev, &QIODevice::readyRead, [this](){
        if(inpDev != nullptr && outDev != nullptr){
            if(call != nullptr && call->getSocket()->isOpen()){
                QByteArray bArr = inpDev->readAll();
                call->sendData(bArr);
            }
        }
    });
    bool connected = connect(call->getSocket(), &QUdpSocket::readyRead, [this](){
        qDebug() << "0-ok";
        if(inpDev != nullptr && outDev != nullptr){
            qDebug() << "1-ok";
            if(call != nullptr && call->getSocket()->isOpen()){
                while (call->getSocket()->hasPendingDatagrams()) {
                    QByteArray bArr;
                    bArr.resize(call->getSocket()->pendingDatagramSize());
                    QHostAddress sender;
                    quint16 senderPort;
                    call->getSocket()->readDatagram(bArr.data(), bArr.size(), &sender, &senderPort);
                    qDebug() << bArr;
                    outDev->write(bArr);
                }
            }
        }
    });
    if(!connected){
        qDebug() << "sorry not connected";
    }
    ui->ipLineEdit->setDisabled(true);
    ui->portLineEdit->setDisabled(true);
    ui->callButton->hide();
    ui->endButton->show();
    ui->peerIPLabel->show();
    ui->peerIPLineEdit->show();
    ui->addPeerButton->show();
    ui->remPeerButton->show();
    ui->peersLabel->show();
    ui->treeWidget->show();
    if(port == 0){
        ui->portLabel->show();
        ui->portLineEdit->show();
        ui->peerPortLabel->show();
        ui->peerPortLineEdit->show();
    }
}


void WannaCall::on_endButton_clicked()
{
    call->getSocket()->close();
    qDebug() << "Successfully closed socket";
    call->getPeers().clear();
    qDebug() << "Cleared peers!";

    ui->ipLineEdit->setDisabled(false);
    ui->portLineEdit->setDisabled(false);

    ui->callButton->show();
    ui->endButton->hide();

    ui->peerIPLabel->hide();

    ui->peerIPLineEdit->hide();
    ui->peerIPLineEdit->clear();

    ui->addPeerButton->hide();
    ui->remPeerButton->hide();

    ui->peersLabel->hide();

    ui->treeWidget->hide();
    ui->treeWidget->clear();

    if(port == 0){
        ui->portLabel->hide();

        ui->portLineEdit->hide();
        ui->portLineEdit->clear();

        ui->peerPortLabel->hide();

        ui->peerPortLineEdit->hide();
        ui->peerPortLineEdit->clear();
    }
    delete call;
    call = nullptr;

    if(inpDev != nullptr){
        inpDev->disconnect();
        input->stop();
        inpDev = nullptr;
    }
    if(outDev != nullptr){
        outDev->disconnect();
        output->stop();
        outDev = nullptr;
    }
}


void WannaCall::on_settingsButton_clicked()
{
    if(this->settings != nullptr){
        if(this->settings->isHidden()){
            this->settings->show();
        }else{
            this->settings->hide();
        }
    }
}

