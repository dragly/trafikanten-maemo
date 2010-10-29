#ifndef DEPARTURES_H
#define DEPARTURES_H

#include <QMainWindow>

#include <QtNetwork>
#include <QtXml>
#include <QtGui>

#include "common.h"

namespace Ui {
    class DeparturesWindow;
}
class DeparturesWindow;

class DepartureListModel : public QAbstractListModel {
    Q_OBJECT
public:
    DepartureListModel(QObject *parent = 0, QList<Departure *>list = QList<Departure *>())
        : QAbstractListModel(parent) {
        departure_list = list;
    }
    int rowCount(const QModelIndex &model = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void doReset();
public slots:

private:
    QList<Departure *> departure_list;
};

class DepartureListDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    DepartureListDelegate(DeparturesWindow *parent)
        : QStyledItemDelegate((QObject*)parent) {departuresWindow = parent;}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index);
    DeparturesWindow* departuresWindow;
};

class DeparturesWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeparturesWindow(Place place, QWidget *parent = 0);
    ~DeparturesWindow();

    void refreshData();
    DepartureListDelegate* delegate;

    bool portraitMode() {return _portraitMode;}

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DeparturesWindow *ui;
    QNetworkAccessManager *manager;
    Place place;
    bool _portraitMode;
    DepartureListModel *model;

private slots:
    void on_refreshButton_clicked();
    void on_actionRefresh_triggered();
    void on_actionAddFavorite_triggered();
    void on_actionRoute_to_triggered();
    void on_actionRoute_from_triggered();
    void replyFinished(QNetworkReply* reply);
    void orientationChanged();
};

#endif // DEPARTURES_H
