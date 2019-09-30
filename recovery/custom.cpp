#include "custom.h"

#include <QDebug>

custom::custom()
{
}

QString custom::read(const char * key)
{
    //Get the map of windows (& calibration)
    QVariantMap map = Json::loadFromFile(":/custom.json").toMap();
    return (map.value(key).toString());
}
