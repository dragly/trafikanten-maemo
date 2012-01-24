#ifndef TRAFIKANTENWINDOW_H
#define TRAFIKANTENWINDOW_H

#include <QMainWindow>
#include <QTimer>
#ifdef Q_WS_MAEMO5
#include <qnmeapositioninfosource.h>
#include <qgeopositioninfosource.h>
#include <qgeosatelliteinfosource.h>
#include <qgeopositioninfo.h>
#include <qgeosatelliteinfo.h>
using namespace QtMobility;
#endif
class SearchDialog;
class TravelSearchWindow;
namespace Ui {
    class TrafikantenWindow;
}

class TrafikantenWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TrafikantenWindow(QWidget *parent = 0);
    ~TrafikantenWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TrafikantenWindow *ui;
    QGeoPositionInfoSource *positionSource;
    bool portraitMode;
    SearchDialog* search;
    TravelSearchWindow* travelSearch;
    bool positionSearchPerformed;
    QTime lastPositionSearch;
    QTimer updateRequestTimer;

private slots:
    void on_btnFavorites_clicked();
    void on_btnRecent_clicked();
    void on_actionAbout_triggered();
    void on_btnRouting_clicked();
    void on_btnNearby_clicked();
    void on_btnSearch_clicked();
    void positionUpdated(const QGeoPositionInfo &info);
    void updateTimeout();
    void orientationChanged();
};

#endif // TRAFIKANTENWINDOW_H
