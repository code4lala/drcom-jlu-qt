#include "dogcom.h"

#include <QDebug>
#include "constants.h"

DogCom::DogCom(InterruptibleSleeper *s)
{
    sleeper=s;
}

void DogCom::Stop()
{
    sleeper->Interrupt();
}

void DogCom::FillConfig(QString a, QString p, QString m)
{
    account=a;password=p;mac_addr=m;
}

void DogCom::run()
{
    qDebug()<<"Start dogcoming...";
    // 后台登录维持连接的线程
    // 一旦离线就return
    qDebug()<<"Sending handshake...";
//    if(!account.compare("1")&&!password.compare("1")&&!mac_addr.compare("F4:8E:38:E8:46:72")){
//    if(!account.compare("1")&&!password.compare("1")&&!mac_addr.compare("1A:2B:3C:4D:5E:6F")){
//    if(!account.compare("1")&&!password.compare("1")&&!mac_addr.compare("1A:2B:3C:4D:5E:61")){
    if(!account.compare("1")&&!password.compare("1")&&!mac_addr.compare("AC:2B:6E:A2:1B:38")){
        // 登录成功
        qDebug()<<"Logged in.";
        emit ReportOnline();
    }else{
        // 登录失败 因为没有Interrupt，但是后边也要重用这个睡眠定时器，所以此处手动interrupt一下，使登录操作保持一致
        sleeper->Interrupt();
        qDebug()<<"Login failed";
        emit ReportOffline(OFF_WRONG_PASS);
        return;
    }

    qDebug()<<"sending packets cyclically";
    int i=0;
    while(true){
        qDebug()<<QString("in MyThread: %1").arg(i++);
        if(sleeper->Sleep(2000)){
            //睡眠成功
        }else{
            qDebug()<<"Interruptted by user";
            emit ReportOffline(OFF_USER_LOGOUT);
            return;
        }
    }
}
