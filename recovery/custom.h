#ifndef CUSTOM_H
#define CUSTOM_H

#include "json.h"

#include <QString>
#include <QVariantMap>

class custom
{
public:
    custom();
    static QString read(const char * key);
};

#endif // CUSTOM_H
