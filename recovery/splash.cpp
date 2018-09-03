#include "splash.h"
#include "mydebug.h"

#include <QApplication>
#include <QDesktopWidget>

KSplash::KSplash(const QPixmap &pixmap, Qt::WindowFlags f, bool bresize)
    : QSplashScreen(pixmap, f)
{
    _resize = bresize;
    original_pixmap = pixmap;
    resize();

    QDesktopWidget * desktop = QApplication::desktop();
    connect(desktop, SIGNAL(workAreaResized(int)), this, SLOT(resize()));
}

KSplash::KSplash(QWidget *parent, const QPixmap &pixmap, Qt::WindowFlags f, bool bresize)
    : QSplashScreen(parent, pixmap, f)
{
    _resize = bresize;
    original_pixmap = pixmap;
    resize();

    QDesktopWidget * desktop = QApplication::desktop();
    connect(desktop, SIGNAL(workAreaResized(int)), this, SLOT(resize()));
}

void KSplash::mousePressEvent(QMouseEvent *)
{
}

void KSplash::resize()
{
    if (_resize)
    {
        QPixmap newpix = original_pixmap.scaled( QApplication::desktop()->screenGeometry(-1).size() );
        setPixmap(newpix);
    }
    else
    {
        setPixmap(original_pixmap);
    }
}
