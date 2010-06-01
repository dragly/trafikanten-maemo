#include "routesearchwindow.h"
#include "ui_routesearchwindow.h"

RouteSearchWindow::RouteSearchWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RouteSearchWindow)
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
#endif
    ui->setupUi(this);
}

void RouteSearchWindow::setPlace(Place place, bool isFrom) {
    if(isFrom) {
        this->placeFrom = place;
        ui->btnPlaceFrom->setText(place.placeName);
    } else {
        this->placeTo = place;
        ui->btnPlaceTo->setText(place.placeName);
    }
}

RouteSearchWindow::~RouteSearchWindow()
{
    delete ui;
}

void RouteSearchWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
