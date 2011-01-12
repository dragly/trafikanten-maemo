#ifndef ROUTESEARCHWINDOW_H
#define ROUTESEARCHWINDOW_H

#include <QtNetwork>
#include <QtGui>
#ifdef Q_WS_MAEMO_5
#include <QMaemo5DatePickSelector>
#include <QMaemo5TimePickSelector>
#include <QMaemo5ValueButton>
#endif

#include "common.h"

class SearchDialog;

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

    enum FavoriteSelectMode {FavoriteTo, FavoriteFrom};

    int favoriteSelectMode;

    void setPlace(Place place, bool isFrom);
    void setPlaceFrom(Place place);
    void setPlaceTo(Place place);
    void updateButtonText();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TravelSearchWindow *ui;
    Place placeFrom;
    Place placeTo;
    bool portraitMode;
    QNetworkAccessManager *manager;
    Place searchPlace(SearchDialog *search);
#ifdef Q_WS_MAEMO_5
    QMaemo5DatePickSelector *datePicker;
    QMaemo5TimePickSelector *timePicker;
#endif
    SearchDialog *searchFromDialog;
    SearchDialog *searchToDialog;
    void showFavoriteMessage(Place place, int mode);
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *dateButton;
    QMaemo5ValueButton *timeButton;
#endif

private slots:
    void on_btnNow_clicked();
    void on_btnToFavorite_clicked();
    void on_btnFromFavorite_clicked();
    void on_actionSwitch_direction_triggered();
    void on_actionAddFavorite_triggered();
    void on_tblResults_clicked(QModelIndex index);
    void on_pushButton_clicked();
    void on_btnPlaceTo_clicked();
    void on_btnPlaceFrom_clicked();
    void orientationChanged();
    void replyFinished(QNetworkReply *reply);
    void favoritePlaceSelected(Place place);
};

#endif // ROUTESEARCHWINDOW_H
