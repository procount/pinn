#include "progressslideshowdialog.h"
#include "ui_progressslideshowdialog.h"
#include "util.h"
#define DBG_LOCAL 1
#define LOCAL_DBG_ON 0
#define LOCAL_DBG_FUNC 0
#define LOCAL_DBG_OUT 0
#define LOCAL_DBG_MSG 0
#include "mydebug.h"

#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QDesktopWidget>
#include <QDebug>
#include <QProcess>

/* Progress dialog with slideshow
 *
 * Initial author: Floris Bos
 * Maintained by ProCount
 *
 * See LICENSE.txt for license details
 *
 */

//@@ Constructor with parameters & initiliasers
ProgressSlideshowDialog::ProgressSlideshowDialog(const QStringList &slidesDirectories, const QString &statusMsg, int changeInterval, QWidget *parent) :
    QDialog(parent),
    _drive(""),
    _pos(0),
    _changeInterval(changeInterval),
    _maxSectors(0),
    _pausedAt(0),
    _totalSize(0),
    _accountMode(ePM_NONE),
    ui(new Ui::ProgressSlideshowDialog)
{
    TRACE
    ui->setupUi(this);
    setLabelText(statusMsg);

    ui->imagespace->setScaledContents(true); //Scale all slides to be the same size

    QRect s = QApplication::desktop()->screenGeometry();
    if (s.height() < 400)
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    else
        setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    foreach (QString slidesDirectory, slidesDirectories)
    {
        QDir dir(slidesDirectory, "*.jpg *.jpeg *.png");
        if (dir.exists())
        {
            QStringList s = dir.entryList();
            s.sort();

            foreach (QString slide, s)
            {
                _slides.append(slidesDirectory+"/"+slide);
            }
        }
    }
    //qDebug() << "Available slides" << _slides;

    if (_slides.isEmpty())
    {
        /* Resize window to just show progress bar */
        ui->imagespace->setMinimumSize(0, 0);
        resize(this->width(), 50);

    }
    else
    {
        /* Resize window to size of largest image in slide directory */
        int maxwidth=0;
        int maxheight=0;
        foreach (QString slide, _slides)
        {   //Get largest slide dimension
            QPixmap pix(slide);
            maxwidth = qMax(maxwidth, pix.width());
            maxheight = qMax(maxheight, pix.height());
        }
        maxheight += ui->frame->height(); //89 for lower progress bar
        //Ensure it is smaller than physical screen
        maxwidth = qMin(maxwidth, s.width()-10);
        maxheight = qMin(maxheight, s.height()-100);
        //Resize dialog box. imagesize will exapnd to fit
        resize(maxwidth, maxheight);

        QPixmap pixmap(_slides.first());

        ui->imagespace->setPixmap(pixmap);

        connect(&_timer, SIGNAL(timeout()), this, SLOT(nextSlide()));
        _timer.start(changeInterval * 1000);
    }
    connect(&_iotimer, SIGNAL(timeout()), this, SLOT(updateIOstats()));

    //enableIOaccounting();
    _t1.start();
    QProcess::execute("rm /tmp/progress");
}

//@@ ok
ProgressSlideshowDialog::~ProgressSlideshowDialog()
{
    TRACE
    _iotimer.stop();
    finish();
    delete ui;
}

//@@ ok
void ProgressSlideshowDialog::setLabelText(const QString &text)
{
    TRACE
    QString txt = text;
    txt.replace('\n',' ');
    ui->statusLabel->setText(txt);
    //qDebug() << text;
}

//@@ ok
void ProgressSlideshowDialog::setMBWrittenText(const QString &text)
{
    TRACE
    QString txt = text;
    txt.replace('\n',' ');
    ui->mbwrittenLabel->setText(txt);
    //qDebug() << text;
}

//@@ Manually set progress bar
void ProgressSlideshowDialog::updateProgress(qint64 value)
{
    TRACE
    int fraction = (int)(value>>9);
    ui->progressBar->setValue(fraction);
    //qDebug() << "updateProgress " << fraction;
}

//@@ ok
void ProgressSlideshowDialog::nextSlide()
{
    TRACE
    if (++_pos >= _slides.size())
        _pos = 0;

    QString newSlide = _slides.at(_pos);
    if (QFile::exists(newSlide))
        ui->imagespace->setPixmap(QPixmap(newSlide));
}

/* IO accounting functionality for analyzing SD card write speed / showing progress */

//@@ Setup total size
void ProgressSlideshowDialog::setMaximum(qint64 bytes)
{
    TRACE
    /* restrict to size of 1TB since the progressbar expects an int32 */
    /* to prevent overflow */
    if (bytes > 1099511627775LL) /* == 2147483648 * 512 -1*/
        bytes = 1099511627775LL;
    _maxSectors = bytes/512;
    ui->progressBar->setMaximum(_maxSectors);
}

//@@ Setup parameters for accounting for this partition
void ProgressSlideshowDialog::setDriveMode(const QString &drive, eProgressMode mode)
{
    TRACE
    _drive = drive;
    _accountMode = mode;
}

//@@ Start accounting
void ProgressSlideshowDialog::startAccounting()
{
    TRACE
    _sectorsStart = sectorsAccessed();
    _iotimer.start(1000);
}

//@@ Stop accounting
void ProgressSlideshowDialog::stopAccounting()
{
    TRACE
    _iotimer.stop();
}

void ProgressSlideshowDialog::idle()
{
    TRACE
    ui->progressBar->setMaximum(0);
}

void ProgressSlideshowDialog::cont()
{
    TRACE
    ui->progressBar->setMaximum(_maxSectors);
}

//@@ Consolidate
void ProgressSlideshowDialog::consolidate()
{
    TRACE
    _totalSize += sectorsAccessed()-_sectorsStart;
    _sectorsStart = sectorsAccessed();
}


void ProgressSlideshowDialog::finish()
{
    TRACE
    //Force progress bar and /tmp/progress to display 100%
    _totalSize = _maxSectors;
    updateIOstats();
}

//@@ Update stats
void ProgressSlideshowDialog::updateIOstats()
{
    TRACE
    static int last_percent=-1;
    uint sectors = _totalSize + sectorsAccessed()-_sectorsStart;

    double sectorsPerSec = sectors * 1000.0 / _t1.elapsed();
    if (_maxSectors)
    {
        sectors = qMin(_maxSectors, sectors);
        ui->progressBar->setValue(sectors);

        double secondsleft = ((double)_maxSectors - (double)sectors)/sectorsPerSec;
        uint remaining = (uint) secondsleft;
        uint secs = remaining %60;
        remaining /=60; //mins
        uint mins = remaining %60;
        remaining /=60; //hours
        uint hrs = remaining ;

        QString mode;
        if (_accountMode==ePM_READSTATS)
            mode=tr("%1 MB of %2 MB read (%3 MB/sec) Remaining: %4:%5:%6");
        else
            mode=tr("%1 MB of %2 MB written (%3 MB/sec) Remaining: %4:%5:%6");

        setMBWrittenText(mode
            .arg(QString::number(sectors/2048))
            .arg(QString::number(_maxSectors/2048))
            .arg(QString::number(sectorsPerSec/2048.0, 'f', 1))
            .arg(hrs,2,10,QLatin1Char( '0' ))
            .arg(mins,2,10,QLatin1Char( '0' ))
            .arg(secs,2,10,QLatin1Char( '0' )));

        int percent;
        if (_maxSectors>0)
                percent = (100*sectors)/_maxSectors;
        else
                percent = last_percent;
        if (last_percent != percent)
        {
            last_percent=percent;
            QString progress = QString("%1 %\n").arg(QString::number(percent));
            QByteArray output= progress.toUtf8();
            QFile f("/tmp/progress");
            f.open(f.Append);
            f.write(output);
            f.close();

        }
    }
    else
    {
        QString mode;
        if (_accountMode==ePM_READSTATS)
            mode=tr("%1 MB read (%2 MB/sec)");
        else
            mode=tr("%1 MB written (%2 MB/sec)");

        setMBWrittenText(mode
            .arg(QString::number(sectors/2048), QString::number(sectorsPerSec/2048.0, 'f', 1)));

        int percent = sectors/2048;
        if (last_percent != percent)
        {
            last_percent=percent;
            QString progress = QString("%1 MB\n").arg(QString::number(percent));
            QByteArray output= progress.toUtf8();
            QFile f("/tmp/progress");
            f.open(f.Append);
            f.write(output);
            f.close();
        }
    }
}

//@@ Get current progress
uint ProgressSlideshowDialog::sectorsAccessed()
{
    TRACE
    /* Poll kernel counters to get number of bytes written
     *
     * Fields available in /sys/block/<DEVICE>/stat
     * (taken from https://www.kernel.org/doc/Documentation/block/stat.txt )
     *
     * Name            units         description
     * ----            -----         -----------
     * read I/Os       requests      number of read I/Os processed
     * read merges     requests      number of read I/Os merged with in-queue I/O
     * read sectors    sectors       number of sectors read
     * read ticks      milliseconds  total wait time for read requests
     * write I/Os      requests      number of write I/Os processed
     * write merges    requests      number of write I/Os merged with in-queue I/O
     * write sectors   sectors       number of sectors written
     * write ticks     milliseconds  total wait time for write requests
     * in_flight       requests      number of I/Os currently in flight
     * io_ticks        milliseconds  total time this block device has been active
     * time_in_queue   milliseconds  total wait time for all requests
     */

    static uint numsectors=0;

    if (_accountMode==ePM_WRITEDF)
    {
        int errorcode;
        QString cmd = "sh -c \"df -k | grep "+_drive+" | sed -e 's/ \\+/ /g' | cut -d ' ' -f 3\"";
        QString result =readexec(false, cmd, errorcode);
        if (!result.isEmpty())
            numsectors = 2* result.toUInt();
    }
    else if (_accountMode!=ePM_NONE)
    {
        int field=6;
        if (_accountMode==ePM_READSTATS)
            field=2;

        QFile f(sysclassblock(_drive)+"/stat");
        f.open(f.ReadOnly);
        QByteArray ioline = f.readAll().simplified();
        f.close();

        QList<QByteArray> stats = ioline.split(' ');

        if (stats.count() >= field)
            numsectors = stats.at(field).toUInt(); /* Read or write sectors */
    }

    if (numsectors > 2147483647)        //Maybe use MAX_INT from limits.h?
       numsectors = 2147483647;
    return numsectors;
}
