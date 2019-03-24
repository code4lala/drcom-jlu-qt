#include "interruptiblesleeper.h"
#include <QDebug>

InterruptibleSleeper::InterruptibleSleeper(QObject *parent) : QObject(parent)
{
	qDebug() << "InterruptibleSleeper Constructor";
}

bool InterruptibleSleeper::Sleep(int timeout)
{
	m.tryLock();
	if (m.tryLock(timeout)) {
		//获取到锁 睡眠失败 被中断了
		return false;
	}
	else {
		//未获取到锁 睡眠成功 没有被中断
		m.unlock();
		return true;
	}
}

void InterruptibleSleeper::Interrupt() {
	qDebug() << "InterruptibleSleeper Interruptted";
	m.unlock();
}
