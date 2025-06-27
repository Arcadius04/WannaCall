#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);

    connect(this, SIGNAL(outputUpdate(QString)),parent, SLOT(outputUpdate(QString)));
    connect(this, SIGNAL(inputUpdate(QString)),parent, SLOT(inputUpdate(QString)));
    connect(this, SIGNAL(listenUpdate()),parent, SLOT(listenUpdate()));
    setWindowFlags(Qt::Window);
    setWindowTitle("Wanna Call/Settings");
    refreshInputList();
    refreshOutputList();
}

Settings::~Settings()
{
    delete ui;
}

void Settings::refreshInputList(){
    ui->inputList->clear();
    QList<QAudioDevice> inpDevices = QMediaDevices::audioInputs();
    for(QAudioDevice dev : inpDevices){
        ui->inputList->addItem(dev.description());
    }
}
void Settings::refreshOutputList(){
    ui->outputList->clear();
    QList<QAudioDevice> outDevices = QMediaDevices::audioOutputs();
    for(QAudioDevice dev : outDevices){
        ui->outputList->addItem(dev.description());
    }
}

void Settings::on_listenButton_clicked()
{
    int outRet = emit outputUpdate(ui->outputList->currentText());
    int inRet = emit inputUpdate(ui->inputList->currentText());
    int ret = emit listenUpdate();
}


void Settings::on_refreshButton_clicked()
{
    refreshInputList();
    refreshOutputList();
}


void Settings::on_setButton_clicked()
{
    int outRet = emit outputUpdate(ui->outputList->currentText());
    int inRet = emit inputUpdate(ui->inputList->currentText());
}

