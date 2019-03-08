#ifndef INTERRUPTIBLESLEEPER_H
#define INTERRUPTIBLESLEEPER_H

#include <QObject>
#include <QMutex>

/**
 * @brief The InterruptibleSleeper class
 * 使用说明：先Reset，然后Sleep指定时长即可，单位是ms
 * 中止后台时调用Interrupt，会立即响应
 * Interrupt之后想重新用，再Reset一下，然后Sleep指定时长即可
 */
class InterruptibleSleeper : public QObject
{
    Q_OBJECT
public:
    explicit InterruptibleSleeper(QObject *parent = nullptr);
    // 睡眠成功返回 true 被中断返回 false
    bool Sleep(int timeout);
    void Reset();
    void Interrupt();

private:
    QMutex m;
};

#endif // INTERRUPTIBLESLEEPER_H
