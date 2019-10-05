#include "splash.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>

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

void KSplash::drawContents(QPainter *painter)
{
    QPixmap textPix = QSplashScreen::pixmap();
    painter->setPen(this->color);
    painter->drawText(this->rect, this->alignement, this->message);
}

void KSplash::showStatusMessage(const QString &message, const QColor &color)
{
    this->message = message;
    this->color = color;
    this->showMessage(this->message, this->alignement, this->color);
}

void KSplash::setMessageRect(QRect rect, int alignement)
{
    this->rect = rect;
    this->alignement = alignement;
}

#if 0
Snippet how to create a splashscreen with opacity and dynamic text.


In main.cpp:
------------

QPixmap splashImage(":images/splash.png");
QPixmap splashMask(":images/splashmask.png");

customSplashScreen* splash = new customSplashScreen(splashImage);
splash->setMessageRect(QRect::QRect(7, 253, 415, 14), Qt::AlignCenter); // Setting the message position.

QFont splashFont;
splashFont.setFamily("Arial");
splashFont.setBold(true);
splashFont.setPixelSize(9);
splashFont.setStretch(125);

splash->setFont(splashFont);
splash->setMask(splashMask);
splash->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SplashScreen);
splash->show();

/* To intercept mousclick to hide splash screen. Since the
splash screen is typically displayed before the event loop
has started running, it is necessary to periodically call. */
app.processEvents();

splash->showStatusMessage(QObject::tr("Initializing…"));

/* Some code here */

app.processEvents();

splash->showStatusMessage(QObject::tr("Loading something…"));

customSplashScreen.h:
--------------------

#ifndef CUSTOMSPLASHSCREEN_H
#define CUSTOMSPLASHSCREEN_H

#include <QSplashScreen>
#include <QPainter>

class customSplashScreen : public QSplashScreen
{
public:
    customSplashScreen(const QPixmap& pixmap);
    ~customSplashScreen();
    virtual void drawContents(QPainter *painter);
    void showStatusMessage(const QString &message, const QColor &color = Qt::black);
    void setMessageRect(QRect rect, int alignment = Qt::AlignLeft);
private:
    QString message;
    int alignement;
    QColor color;
    QRect rect;
};

#endif // CUSTOMSPLASHSCREEN_H

customSplashScreen.cpp:
-----------------------

#include "customSplashScreen.h"

customSplashScreen::customSplashScreen(const QPixmap& pixmap)
{
   QSplashScreen::setPixmap(pixmap);
};

customSplashScreen::~customSplashScreen()
{
};

void customSplashScreen::drawContents(QPainter *painter)
{
    QPixmap textPix = QSplashScreen::pixmap();
    painter->setPen(this->color);
    painter->drawText(this->rect, this->alignement, this->message);
};

void customSplashScreen::showStatusMessage(const QString &message, const QColor &color)
{
    this->message = message;
    this->color = color;
    this->showMessage(this->message, this->alignement, this->color);
};

void customSplashScreen::setMessageRect(QRect rect, int alignement)
{
    this->rect = rect;
    this->alignement = alignement;
};
#endif
