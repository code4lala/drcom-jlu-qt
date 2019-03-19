#include "interruptiblesleeper.h"
#include <QDebug>

InterruptibleSleeper::InterruptibleSleeper(QObject *parent) : QObject(parent)
{
    qDebug()<<"InterruptibleSleepe Constructor";
}

bool InterruptibleSleeper::Sleep(int timeout)
{
    qDebug()<<"tryLock...";
    m.tryLock();
    qDebug()<<"locked!";
    qDebug()<<"tryLock for"<<timeout<<"millseconds...";
    if(m.tryLock(timeout)){
        qDebug()<<"Interrupted! lock succeed";
        //获取到锁 睡眠失败 被中断了
        return false;
    }else{
        qDebug()<<"lock failed! sleep succeed";
        //未获取到锁 睡眠成功 没有被中断
        m.unlock();
        return true;
    }
}

void InterruptibleSleeper::Interrupt(){
    m.unlock();
}
