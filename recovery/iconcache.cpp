#include "iconcache.h"
#include "json.h"
#include "util.h"

#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QVariantMap>
#include <QDebug>

iconcache::iconcache()
{
    loadCache();
}

iconcache::~iconcache()
{
    storeCache();
}


void iconcache::loadCache()
{
    if (QFile::exists(CACHEFILE))
    {
        _cachemap = Json::loadFromFile(CACHEFILE).toMap();
    }
    else
    {
        QDir dir;
        dir.mkdir(CACHEROOT);
        storeCache();
    }
}


void iconcache::storeCache()
{
    Json::saveToFile(CACHEFILE, _cachemap);
}

void iconcache::storePixmapInCache(QUrl url, QPixmap pix)
{
    QString path= url.toString();
    if (_cachemap.contains(path))
        return;

    QString pixname = CORE(path.mid(path.lastIndexOf('/')+1));
    if (pixname.right(4) != ".png")
        pixname += ".png";
    _cachemap.insert(path,pixname);
    pix.save(CACHEROOT+pixname);
    storeCache();
}

int iconcache::readPixmapFromCache(QUrl url, QPixmap &pix)
{
    QString path= url.toString();
    if (_cachemap.contains(path))
    {
        QString pixname = _cachemap[path].toString();
        if (pix.load(CACHEROOT+pixname))
            return 1;
    }
    return 0;
}
