#ifndef COUNTDOWNFILTER_H
#define COUNTDOWNFILTER_H

/* Stops reboot countdown on an eent
 *
 *
 * See LICENSE.txt for license details
 *
 */

#include <QObject>
#include <QTimer>

class CountdownFilter : public QObject
{
    Q_OBJECT
public:
    explicit CountdownFilter(QObject *parent = 0);
    ~CountdownFilter();
    virtual bool eventFilter(QObject *, QEvent *event);
    void startCountdown(int);
    void stopCountdown(void);
    int getCountdown() { return _countdown; }

signals:
    void countdownTick(int);
    void countdownExpired(void);
    void countdownStopped(void);

public slots:
    void countdown();

protected:
    QTimer * _timer;
    int _countdown;
};

#endif // CountdownFilter_H
