#ifndef RECENTWINDOW_H
#define RECENTWINDOW_H

#include <QtGui>
#include <QtXml>

#include "common.h"

namespace Ui {
    class RecentWindow;
}

class SearchListModel : public QAbstractListModel {
    Q_OBJECT
public:
    SearchListModel(QObject *parent = 0, QList<Search *>list = QList<Search *>())
        : QAbstractListModel(parent) {
        searches = list;
    }
    int rowCount(const QModelIndex &model = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void doReset();
public slots:

private:
    QList<Search *> searches;
};

class SearchListDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    SearchListDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index);
};

class RecentWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Mode {Recent, Favorites};

    explicit RecentWindow(Mode mode, QWidget *parent = 0);
    ~RecentWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::RecentWindow *ui;

    QStandardItemModel* model;
    QSettings settings;
    QList<Search*> searches;

    bool portraitMode;
    int mode;

private slots:
    void on_tblResults_clicked(QModelIndex index);
    void orientationChanged();
};

#endif // RECENTWINDOW_H
