#ifndef WANNACALL_H
#define WANNACALL_H

#include <QMainWindow>
#include <QAudioDevice>
#include <QAudioSource>
#include <QAudioSink>
#include <QMediaDevices>
#include <QMediaRecorder>
#include <QIODevice>
#include <QCryptographicHash>
#include "callobject.h"
#include "settings.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class WannaCall;
}
QT_END_NAMESPACE

class WannaCall : public QMainWindow
{
    Q_OBJECT

public:
    WannaCall(QWidget *parent = nullptr);
    ~WannaCall();
    void refreshPeers();

private slots:
    int inputUpdate(const QString &text);

    int outputUpdate(const QString &text);

    int listenUpdate();

    void on_addPeerButton_clicked();

    void on_remPeerButton_clicked();

    void on_callButton_clicked();

    void on_endButton_clicked();

    void on_settingsButton_clicked();

private:
    Ui::WannaCall *ui;
    QAudioSource* input = nullptr;
    QAudioSink* output = nullptr;
    QIODevice* inpDev = nullptr;
    QIODevice* outDev = nullptr;
    QAudioFormat format;
    CallObject* call = nullptr;
    Settings* settings = nullptr;
    short port = 9999;
    bool listening = false;
};
#endif // WANNACALL_H
