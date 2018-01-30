#include "countdownfilter.h"
#include <QMouseEvent>
#include <QTimer>

/* Stops countdown on any event
 *
 *
 * See LICENSE.txt for license details
 *
 */

CountdownFilter::CountdownFilter(QObject *parent) :
    QObject(parent), _countdown(0)
{
    _timer = new QTimer(this);
}

CountdownFilter::~CountdownFilter()
{
    stopCountdown();
    delete _timer;
}

void CountdownFilter::startCountdown(int timeinsecs)
{
    _countdown=timeinsecs;
    connect(_timer, SIGNAL(timeout()), this, SLOT(countdown()));
    _timer->start(1000);
    emit countdownTick(_countdown);
}

void CountdownFilter::stopCountdown()
{
    if (_countdown)
    {
        _countdown=0;
        emit countdownTick(_countdown);
        emit countdownStopped();
        _timer->stop();
    }
}

void CountdownFilter::countdown()
{
    if (_countdown)
    {
        _countdown--;
        emit countdownTick(_countdown);
        if (_countdown == 0)
        {
            _timer->stop();
            emit countdownExpired();
        }
    }
}

bool CountdownFilter::eventFilter(QObject *, QEvent *event)
{
    if (_countdown)
    {
        if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Enter)
        {
            stopCountdown();
        }
    }
    return false;
}
