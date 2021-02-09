#ifndef PROGRESSSLIDESHOWDIALOG_H
#define PROGRESSSLIDESHOWDIALOG_H

/* Progress dialog with slideshow
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

#include <QProgressDialog>
#include <QTimer>
#include <QTime>

namespace Ui {
class ProgressSlideshowDialog;
}

typedef enum progressmode_tag
{
    ePM_NONE=0,
    ePM_WRITESTATS,
    ePM_WRITEDF,
    ePM_READSTATS
} eProgressMode;

class ProgressSlideshowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressSlideshowDialog(const QStringList &slidesDirectories, const QString &statusMsg = "", int changeInterval = 20, QWidget *parent = 0);
    ~ProgressSlideshowDialog();

public slots:
    void setLabelText(const QString &text);
    void setMBWrittenText(const QString &text);
    void updateProgress(qint64 value);
    void nextSlide();
    void setMaximum(qint64 bytes);
    void setDriveMode(const QString &drive, eProgressMode mode);
    void startAccounting();
    void stopAccounting();
    void idle();
    void cont();
    void consolidate();
    void finish();
    void updateIOstats();
    void reject();

protected:
    QString _drive;
    QStringList _slides;
    int _pos, _changeInterval;
    uint _sectorsStart, _maxSectors, _pausedAt, _totalSize;
    QTimer _timer, _iotimer;
    QTime _t1;
    eProgressMode _accountMode;

    uint sectorsAccessed();


private:
    Ui::ProgressSlideshowDialog *ui;
};

#endif // PROGRESSSLIDESHOWDIALOG_H
