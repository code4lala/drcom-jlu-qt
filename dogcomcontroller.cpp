#include "dogcomcontroller.h"
#include <QDebug>

DogcomController::DogcomController()
{
    sleeper=new InterruptibleSleeper();
    dogcom=new DogCom(sleeper);

    connect(dogcom,&DogCom::ReportOnline,this,&DogcomController::HandleDogcomOnline);
    connect(dogcom,&DogCom::ReportOffline,this,&DogcomController::HandleDogcomOffline);
}

void DogcomController::Login(const QString &account,const QString &password,const QString &mac_addr){
    qDebug()<<"Resetting sleeper...";
    sleeper->Reset();
    qDebug()<<"Reset sleeper done.";
    qDebug()<<"Filling config...";
    dogcom->FillConfig(account,password,mac_addr);
    qDebug()<<"Fill config done.";
    dogcom->start();
}

void DogcomController::LogOut()
{
    dogcom->Stop();
}

void DogcomController::HandleDogcomOffline(int reason)
{
    emit HaveBeenOffline(reason);
}

void DogcomController::HandleDogcomOnline()
{
    emit HaveLoggedIn();
}
