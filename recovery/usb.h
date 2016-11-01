#ifndef USB_H
#define USB_H

#include <QObject>
#include <QTimer>

class usb : public QObject
{
    Q_OBJECT
public:
    explicit usb(QObject *parent = 0);

public:
    void startMonitoringDrives(void);
    void stopMonitoringDrives(void);

signals:
    void drivesChanged();

private slots:
    void checkDrives(void);

private:
    QTimer *timer;

};

#endif // USB_H
