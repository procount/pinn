#ifndef ICONCACHE_H
#define ICONCACHE_H

#include <QPixmap>
#include <QUrl>
#include <QVariantMap>

#define CACHEROOT "/settings/iconcache/"
#define CACHEFILE "/settings/iconcache/cachemap.json"

class iconcache
{
public:
    iconcache();
    ~iconcache();

    void storePixmapInCache(QUrl url, QPixmap pix);
    int readPixmapFromCache(QUrl url, QPixmap &pix);
    void loadCache();
    void storeCache();

private:
    QVariantMap _cachemap;

};

#endif // ICONCACHE_H
