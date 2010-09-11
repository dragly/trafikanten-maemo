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
        searches_ = list;
    }
    int rowCount(const QModelIndex &model = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void doReset();
    void setSearches(QList<Search *> searches) { this->searches_ = searches; }
    QList<Search*> searches() { return searches_; }
public slots:

private:
    QList<Search *> searches_;
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
    enum Mode {Recent, Favorites, FavoritesRealtime};

    explicit RecentWindow(Mode mode, QWidget *parent = 0);
    ~RecentWindow();

protected:
    void changeEvent(QEvent *e);

signals:
    void placeSelected(Place place);

private:
    Ui::RecentWindow *ui;

    SearchListModel *model;
    QSettings settings;

    bool portraitMode;
    int mode;

private slots:
    void on_actionDelete_all_triggered();
    void on_tblResults_customContextMenuRequested(QPoint pos);
    void on_tblResults_clicked(QModelIndex index);
    void orientationChanged();
    void removeFavorite();
};

#endif // RECENTWINDOW_H
