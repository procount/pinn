#ifndef DRVMON_H
#define DRVMON_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <QList>

class drvmon : public QObject
{
    Q_OBJECT
public:
    explicit drvmon(QObject *parent = 0);
    ~drvmon();

public:
    void startMonitoringDrives(int period=5000);
    void stopMonitoringDrives(int period=5000);
    inline QStringList getDriveList()
    {
        return (_driveList);
    }


signals:
    void drivesChanged(void);

protected:
    void calcMinPeriod(void);

private slots:
    void checkDrives(void);

private:
    QTimer      *_timer;
    QStringList _driveList;
    int         _shortestPeriodms;
    QList<int>  _periodList;
};

#endif // DRVMON_H
