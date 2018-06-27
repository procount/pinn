#ifndef PICLONEDIALOG_H
#define PICLONEDIALOG_H

#include <QDialog>

namespace Ui {
class piclonedialog;
}

class piclonedialog : public QDialog
{
    Q_OBJECT

public:
    explicit piclonedialog(QWidget *parent = 0);
    ~piclonedialog();
    inline QString get_src() {return _src;}
    inline QString get_dst() {return _dst;}
    inline QString get_src_dev() {return src_dev;}
    inline QString get_dst_dev() {return dst_dev;}
    inline bool get_resize() {return _resize;}

protected:
    int get_dev_name(const char *dev, char *name);
    void on_cb_Changed(const QString &arg1);
    void on_drives_changed(void);
    void startMonitoringDrives(void);
    void stopMonitoringDrives(void);
private slots:
    void checkDrives(void);
    void on_from_cb_currentIndexChanged(const QString &arg1);
    void on_to_cb_currentIndexChanged(const QString &arg1);
    void on_buttonBox_accepted();

private:
    Ui::piclonedialog *ui;
    int src_count;
    int dst_count;
    bool _resize;
    QString src_dev;
    QString dst_dev;
    QString _src;
    QString _dst;
    QTimer *timer;
};

#endif // PICLONEDIALOG_H
