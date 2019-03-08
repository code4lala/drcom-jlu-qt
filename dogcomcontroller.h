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
signals:
    void HaveBeenOffline(int reason);
    void HaveLoggedIn();
private:
    InterruptibleSleeper *sleeper;
    DogCom *dogcom;
};

#endif // DOGCOMCONTROLLER_H
