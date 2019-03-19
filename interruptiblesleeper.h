#ifndef INTERRUPTIBLESLEEPER_H
#define INTERRUPTIBLESLEEPER_H

#include <QObject>
#include <QMutex>

/**
 * @brief The InterruptibleSleeper class
 * 使用说明：直接Sleep指定时长即可，单位是ms
 * 中止后台时调用Interrupt，会立即响应
 * 通过判断Sleep的返回值确定是否被中断，true即睡眠成功未被中断，false即被中断
 */
class InterruptibleSleeper : public QObject
{
    Q_OBJECT
public:
    explicit InterruptibleSleeper(QObject *parent = nullptr);
    // 睡眠成功返回 true 被中断返回 false
    bool Sleep(int timeout);
    void Interrupt();

private:
    QMutex m;
};

#endif // INTERRUPTIBLESLEEPER_H
