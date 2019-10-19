#ifndef INPUT_H
#define INPUT_H

#include <QThread>

class input : public QThread
{
    Q_OBJECT
public:
    explicit input(QObject *parent = 0);

signals:

public slots:

};

#endif // INPUT_H
