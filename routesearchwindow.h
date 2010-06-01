#ifndef ROUTESEARCHWINDOW_H
#define ROUTESEARCHWINDOW_H

#include <QMainWindow>

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
};

#endif // ROUTESEARCHWINDOW_H
