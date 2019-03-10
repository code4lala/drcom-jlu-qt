#include "interruptiblesleeper.h"

InterruptibleSleeper::InterruptibleSleeper(QObject *parent) : QObject(parent)
{
}

bool InterruptibleSleeper::Sleep(int timeout)
{
    return !m.tryLock(timeout);
    //获取到锁 睡眠失败 被中断了
    //未获取到锁 睡眠成功 没有被中断
}

void InterruptibleSleeper::Reset(){
    m.lock();
}

void InterruptibleSleeper::Interrupt(){
    m.unlock();
}
