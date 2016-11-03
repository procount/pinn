#ifndef USB_H
#define USB_H

#include <QObject>
#include <QTimer>
#include <QStringList>

class usb : public QObject
{
    Q_OBJECT
public:
    explicit usb(QObject *parent = 0);

public:
    void startMonitoringDrives(void);
    void stopMonitoringDrives(void);
    inline QStringList getDriveList()
    {
        return (_driveList);
    }

signals:
    void drivesChanged();

private slots:
    void checkDrives(void);

private:
    QTimer      *_timer;
    QStringList _driveList;
};

#endif // USB_H
