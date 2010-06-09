#ifndef ROUTESEARCHWINDOW_H
#define ROUTESEARCHWINDOW_H

#include <QtNetwork>
#include <QtGui>

#include "common.h"

namespace Ui {
    class RouteSearchWindow;
}

class RouteSearchWindow : public QMainWindow {
    Q_OBJECT
public:
    RouteSearchWindow(QWidget *parent = 0);
    ~RouteSearchWindow();

    void setPlace(Place place, bool isFrom);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::RouteSearchWindow *ui;
    Place placeFrom;
    Place placeTo;
    bool portraitMode;
    QNetworkAccessManager *manager;
    QStandardItemModel *model;
    Place searchPlace();

private slots:
    void on_pushButton_clicked();
    void on_btnPlaceTo_clicked();
    void on_btnPlaceFrom_clicked();
    void orientationChanged();
    void replyFinished(QNetworkReply *reply);
};

#endif // ROUTESEARCHWINDOW_H
