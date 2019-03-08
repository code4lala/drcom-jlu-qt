#ifndef DOGCOM_H
#define DOGCOM_H

#include <QThread>
#include "interruptiblesleeper.h"

class DogCom : public QThread
{
    Q_OBJECT
public:
    DogCom(InterruptibleSleeper *);
    void Stop();
    void FillConfig(QString a,QString p,QString m);
protected:
    void run();
private:
    InterruptibleSleeper *sleeper;
    QString account;
    QString password;
    QString mac_addr;
signals:
    void ReportOffline(int reason);
    void ReportOnline();
};

#endif // DOGCOM_H
