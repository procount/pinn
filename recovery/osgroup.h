#ifndef OSGROUP_H
#define OSGROUP_H

#include "ui_mainwindow.h"
#include "twoiconsdelegate.h"

#include <QObject>
#include <QVariantMap>
#include <QListWidgetItem>
#include <QString>
#include <QMainWindow>
#include <QTabWidget>

//namespace Ui {
//class MainWindow;
//}

class OsGroup : public QObject
{
    Q_OBJECT
public:
    explicit OsGroup(QMainWindow *mw, Ui::MainWindow *ui, bool doGrouping, QObject *parent = 0);

    QMainWindow * _mw;
    Ui::MainWindow *_ui;
    QListWidget * list;
    QListWidget * listInstalled;
    QTabWidget * tabs;
    QVariantMap  osGroupMap;
    bool _bGroup;

    void newTab(const QString &tabName);
    QListWidget * findTab(const QString &tabName);
    void addItem(QListWidgetItem * item);
    void insertItem(int row, QListWidgetItem * item);
    QString getGroup(const QVariantMap& entry);
    void loadMap(const QString &filename);
    void toggleInstalled(bool newState);
    void setFocus(void);

    int count();
    QListWidgetItem *findItemByDataName(const QString &name);
    void updateInstalledStatus();
    QList<QListWidgetItem *> selectedItems();
    QList<QListWidgetItem *> selectedInstalledItems();
    QList<QListWidgetItem *> allItems();
    QList<QListWidgetItem *> findItems ( const QString & text, Qt::MatchFlags flags ) const;
    void setDefaultItems(void);
    void retranslateUI();

#if 0
    //Functions that operate on the whole list of OSes
    clear();
    count();
    findItems(x,y);
    iconSize();
    item(i);
    insertItem();
    addItem();
    setIconSize(x,y);
    update();

    //Functions that operated on the current list
    setItemDelegate();
    installEventFilter();
    setCurrentItem();
    setCurrentRow();
    insertItem();
    addItem();
    currentItem();
    currentIndex();
    update();
#endif

signals:


protected:
    QMap<QString, QListWidget*> tabMap; //"group" -> ui->List

private slots:
    void tabs_currentChanged(int index);
};

#endif // OSGROUP_H
