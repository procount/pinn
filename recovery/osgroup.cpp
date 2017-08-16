#include "config.h"
#include "json.h"
#include "mydebug.h"
#include "osgroup.h"

#include <iterator>
#include <QFile>
#include <QTabWidget>

OsGroup::OsGroup(QMainWindow *mw, Ui::MainWindow *ui, QObject *parent) :
    QObject(parent),
    _mw(mw),
    _ui(ui)
{
    tabs=NULL;
    list=_ui->list;

    listInstalled = new QListWidget();
    _ui->gridLayout_2->addWidget(listInstalled,0,0,1,1);
    listInstalled->setVisible(false);

    QFont font;
    font.setPointSize(12);
    listInstalled->setFont(_ui->list->font());
    listInstalled->setSpacing(_ui->list->spacing());
    listInstalled->setIconSize(QSize(40,40));

    listInstalled->setItemDelegate(new TwoIconsDelegate(_mw)); //PI
    listInstalled->installEventFilter(_mw); //PI

    connect(listInstalled, SIGNAL(currentRowChanged(int)), _mw, SLOT(on_list_currentRowChanged(void)));
    connect(listInstalled, SIGNAL(doubleClicked(const QModelIndex&)), _mw, SLOT(on_list_doubleClicked(const QModelIndex&)));
    connect(listInstalled, SIGNAL(itemChanged(QListWidgetItem *)), _mw, SLOT(on_list_itemChanged(QListWidgetItem *)));
}

void OsGroup::loadMap(const QString &filename)
{
    if (QFile::exists(filename))
    {
        osGroupMap = Json::loadFromFile(filename).toMap();
    }
}


void OsGroup::addItem(QListWidgetItem * item)
{
    QVariantMap entry = item->data(Qt::UserRole).toMap();
    QString name = entry.value("name").toString();
    QString group = getGroup(name);

    if (tabs || group != DEFGROUP)
    {
        list = findTab(group);
        if (!list)
        {
            newTab(group); //changes list
        }
    }
    list->addItem(item);
}

void OsGroup::insertItem(int row, QListWidgetItem * item)
{
    QVariantMap entry = item->data(Qt::UserRole).toMap();
    QString name = entry.value("name").toString();
    QString group = getGroup(name);

    if (tabs || group != DEFGROUP)
    {
        list = findTab(group);
        if (!list)
        {
            newTab(group); //changes list
        }
    }
    list->insertItem(row, item);
}

QString OsGroup::getGroup(const QString& name)
{
    return (osGroupMap.value(name,QString(DEFGROUP)).toString());
}

void OsGroup::newTab(const QString &tabName)
{
    if (!tabs)
    {   // Create a new tabwidget
        tabs = new QTabWidget();
        tabs->setObjectName("tabWidget");

        QString title(tabName);
        _ui->list->setIconSize(QSize(40,40));

        _ui->gridLayout_2->removeWidget(_ui->list);
        _ui->list->close();
        if (_ui->list->count())
        {
            //We already have some OSes in list, so put them in a General tab
            title=DEFGROUP;
        }
        tabs->addTab(_ui->list,title);
        tabMap[title] = _ui->list;
        _ui->gridLayout_2->addWidget(tabs,0,0,1,1);
        connect (tabs, SIGNAL(currentChanged(int)), this, SLOT(tabs_currentChanged(int)));
    }

    list = findTab(tabName);
    if (!list)
    {
        //Create a new list for the tab
        list = new QListWidget();
        QFont font;
        font.setPointSize(12);
        list->setFont(_ui->list->font());
        list->setSpacing(_ui->list->spacing());
        list->setIconSize(QSize(40,40));
        list->setItemDelegate(new TwoIconsDelegate(_mw)); //PI
        list->installEventFilter(_mw); //PI

        connect(list, SIGNAL(currentRowChanged(int)), _mw, SLOT(on_list_currentRowChanged(void)));
        connect(list, SIGNAL(doubleClicked(const QModelIndex&)), _mw, SLOT(on_list_doubleClicked(const QModelIndex&)));
        connect(list, SIGNAL(itemChanged(QListWidgetItem *)), _mw, SLOT(on_list_itemChanged(QListWidgetItem *)));

        //Add the new tab to TW
        tabs->addTab(list,tabName);
        tabMap[tabName] = list;
    }
}

void OsGroup::toggleInstalled(bool newState)
{
    if (tabs)
    {
        tabs->setVisible(!newState);
        if (!newState)
            tabs->currentWidget()->setFocus();
    }
    else
    {
        _ui->list->setVisible(!newState);
        if (!newState)
            _ui->list->setFocus();
    }

    listInstalled->setVisible(newState);
    if (newState)
        listInstalled->setFocus();
}

QListWidget * OsGroup::findTab(const QString &tabName)
{
    if (tabMap.contains(tabName))
    {
        return( tabMap[tabName]);
    }
    else
        return NULL;

}

void OsGroup::tabs_currentChanged(int index)
{
    Q_UNUSED(index);

    list = static_cast<QListWidget*> (tabs->currentWidget());
}

int OsGroup::count()
{
    int count=0;
    if (!tabs)
        return(list->count());

    QMap<QString, QListWidget *>::iterator i;
    for (i = tabMap.begin(); i != tabMap.end(); ++i)
        count += i.value()->count();
    return(count);
}


QList<QListWidgetItem *> OsGroup::selectedItems()
{
    QList<QListWidgetItem *> selected;

    if (!tabs)
    {
        for (int i=0; i < list->count(); i++)//ALL
        {
            QListWidgetItem *item = list->item(i); //ALL
            if (item->checkState())
            {
                selected.append(item);
            }
        }
    }
    else
    {
        QMap<QString, QListWidget *>::iterator l;
        for (l = tabMap.begin(); l != tabMap.end(); ++l)
        {
            for (int i=0; i < l.value()->count(); i++)//ALL
            {
                QListWidgetItem *item = l.value()->item(i); //ALL
                if (item->checkState())
                {
                    selected.append(item);
                }
            }
        }
    }
    return selected;
}

QList<QListWidgetItem *> OsGroup::allItems()
{
    QList<QListWidgetItem *> all;

    if (!tabs)
    {
        for (int i=0; i < list->count(); i++)//ALL
        {
            QListWidgetItem *item = list->item(i); //ALL
            all.append(item);
        }
    }
    else
    {
        QMap<QString, QListWidget *>::iterator l;
        for (l = tabMap.begin(); l != tabMap.end(); ++l)
        {
            for (int i=0; i < l.value()->count(); i++)//ALL
            {
                QListWidgetItem *item = l.value()->item(i); //ALL
                all.append(item);
            }
        }
    }
    return all;
}

QList<QListWidgetItem *> OsGroup::findItems ( const QString & text, Qt::MatchFlags flags ) const
{
    QList<QListWidgetItem *> selected;

    if (!tabs)
    {
        return list->findItems ( text, flags );
    }
    else
    {
        QMap<QString, QListWidget *>::const_iterator l;
        for (l = tabMap.begin(); l != tabMap.end(); ++l)
        {
            selected.append(l.value()->findItems ( text, flags ));
        }
    }
    return (selected);
}

void OsGroup::setDefaultItems()
{
    if (!tabs)
    {
        if (list->count() != 0) //ALL
        {
            QList<QListWidgetItem *> l = list->findItems(RECOMMENDED_IMAGE, Qt::MatchExactly); //ALL

            if (!l.isEmpty())
            {
                list->setCurrentItem(l.first());
            }
            else
            {
                list->setCurrentRow(0);
                list->setCurrentItem(list->item(0));
            }
        }
        list->setFocus();
    }
    else
    {
        QMap<QString, QListWidget *>::iterator l;
        for (l = tabMap.begin(); l != tabMap.end(); ++l)
        {
            if (l.value()->count() != 0)
            {
                QList<QListWidgetItem *> f = l.value()->findItems(RECOMMENDED_IMAGE, Qt::MatchExactly); //ALL

                if (!f.isEmpty())
                {
                    l.value()->setCurrentItem(f.first());
                }
                else
                {
                    l.value()->setCurrentRow(0);
                    l.value()->setCurrentItem(l.value()->item(0));
                }
            }
        }
        tabs->currentWidget()->setFocus();
    }
}

