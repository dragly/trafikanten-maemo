#ifndef ROUTESEARCHWINDOW_H
#define ROUTESEARCHWINDOW_H

#include <QtNetwork>
#include <QtGui>
#include <QMaemo5DatePickSelector>
#include <QMaemo5TimePickSelector>
#include <QMaemo5ValueButton>

#include "common.h"

namespace Ui {
    class TravelSearchWindow;
}

class TravelListModel : public QAbstractListModel {
    Q_OBJECT
public:
    TravelListModel(QObject *parent = 0, QList<Travel *>list = QList<Travel *>())
        : QAbstractListModel(parent) {
        travels = list;
    }
    int rowCount(const QModelIndex &model = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void doReset();
public slots:

private:
    QList<Travel *> travels;
};

class TravelListDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    TravelListDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index);
};

class TravelSearchWindow : public QMainWindow {
    Q_OBJECT
public:
    TravelSearchWindow(QWidget *parent = 0);
    ~TravelSearchWindow();

    void setPlace(Place place, bool isFrom);
    void setPlaceFrom(Place place);
    void setPlaceTo(Place place);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TravelSearchWindow *ui;
    Place placeFrom;
    Place placeTo;
    bool portraitMode;
    QNetworkAccessManager *manager;
    Place searchPlace();
    QMaemo5DatePickSelector *datePicker;
    QMaemo5TimePickSelector *timePicker;

private slots:
    void on_actionAddFavorite_triggered();
    void on_tblResults_clicked(QModelIndex index);
    void on_pushButton_clicked();
    void on_btnPlaceTo_clicked();
    void on_btnPlaceFrom_clicked();
    void orientationChanged();
    void replyFinished(QNetworkReply *reply);
};

#endif // ROUTESEARCHWINDOW_H
