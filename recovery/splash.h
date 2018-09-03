#ifndef KSPLASH_H
#define KSPLASH_H

#include <QSplashScreen>

class KSplash : public QSplashScreen
{
    Q_OBJECT
public:
    explicit KSplash(const QPixmap &pixmap = QPixmap(), Qt::WindowFlags f = 0, bool bresize=false);
    KSplash(QWidget *parent, const QPixmap &pixmap = QPixmap(), Qt::WindowFlags f = 0, bool bresize=false);

protected:
    void mousePressEvent(QMouseEvent *);

signals:

public slots:
    void resize();
private:
    QPixmap original_pixmap;
    bool _resize;

};

#endif // SPLASH_H
