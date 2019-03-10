#ifndef DOGCOMCONTROLLER_H
#define DOGCOMCONTROLLER_H

#include <QApplication>
#include <QObject>
#include "constants.h"
#include "dogcom.h"
#include "interruptiblesleeper.h"

class DogcomController : public QObject
{
    Q_OBJECT
public:
    DogcomController();
    void Login(const QString &account,const QString &password,const QString &mac);
    void LogOut();
public slots:
    void HandleDogcomOffline(int reason);
    void HandleDogcomOnline();
    void HandleIpAddress(unsigned char x1,unsigned char x2,unsigned char x3,unsigned char x4);
signals:
    void HaveBeenOffline(int reason);
    void HaveLoggedIn();
    void HaveObtainedIp(const QString &ip);
private:
    InterruptibleSleeper *sleeper;
    DogCom *dogcom;
};

#endif // DOGCOMCONTROLLER_H
