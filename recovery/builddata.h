#ifndef BUILDDATA_H
#define BUILDDATA_H

#include <QStringList>
#include <QFile>
#include <QString>
#include <QDebug>


class BuildData
{
public:
    BuildData();
    void read(const char * filename);
    bool isEmpty();

    friend bool operator<(const BuildData& l, const BuildData& r);

private:
    QStringList _bdl;
    QString _version;
    QString _date;
};

inline bool operator> (const BuildData& lhs, const BuildData& rhs){ return rhs < lhs; }
inline bool operator<=(const BuildData& lhs, const BuildData& rhs){ return !(lhs > rhs); }
inline bool operator>=(const BuildData& lhs, const BuildData& rhs){ return !(lhs < rhs); }

#endif // BUILDDATA_H
