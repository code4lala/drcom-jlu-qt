#include "interruptiblesleeper.h"

InterruptibleSleeper::InterruptibleSleeper(QObject *parent) : QObject(parent)
{
}

bool InterruptibleSleeper::Sleep(int timeout)
{
    if(m.tryLock(timeout)){
        //获取到锁 睡眠失败 被中断了
        // Interrupt之后此处尝试获取互斥锁成功了，因此还需要再解一下锁
        m.unlock();
        return false;
    }else{
        //未获取到锁 睡眠成功 没有被中断
        return true;
    }
}

void InterruptibleSleeper::Reset(){
    m.lock();
}

void InterruptibleSleeper::Interrupt(){
    m.unlock();
}
