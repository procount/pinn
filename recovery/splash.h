#ifndef KSPLASH_H
#define KSPLASH_H

#include <QSplashScreen>

class KSplash : public QSplashScreen
{
    Q_OBJECT
public:
    explicit KSplash(const QPixmap &pixmap = QPixmap(), Qt::WindowFlags f = 0, bool bresize=false);
    KSplash(QWidget *parent, const QPixmap &pixmap = QPixmap(), Qt::WindowFlags f = 0, bool bresize=false);
    virtual void drawContents(QPainter *painter);
    void showStatusMessage(const QString &message, const QColor &color = Qt::black);
    void setMessageRect(QRect rect, int alignment = Qt::AlignLeft);

protected:
    void mousePressEvent(QMouseEvent *);

signals:

public slots:
    void resize();
private:
    QPixmap original_pixmap;
    bool _resize;
    QString message;
    int alignement;
    QColor color;
    QRect rect;

};

#endif // SPLASH_H
