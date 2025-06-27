#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QAudioDevice>
#include <QMediaDevices>

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();
    void refreshInputList();
    void refreshOutputList();

signals:
    int outputUpdate(const QString &text);
    int inputUpdate(const QString &text);
    int listenUpdate();
private slots:
    void on_listenButton_clicked();

    void on_refreshButton_clicked();

    void on_setButton_clicked();

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H
