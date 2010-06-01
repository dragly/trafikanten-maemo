#ifndef TRAFIKANTENWINDOW_H
#define TRAFIKANTENWINDOW_H

#include <QMainWindow>
#include <qnmeapositioninfosource.h>
#include <qgeopositioninfosource.h>
#include <qgeosatelliteinfosource.h>
#include <qgeopositioninfo.h>
#include <qgeosatelliteinfo.h>
#include <qnetworkconfigmanager.h>
#include <qnetworksession.h>
using namespace QtMobility;
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

private slots:
    void on_btnRouting_clicked();
    void on_btnNearby_clicked();
    void on_btnSearch_clicked();
    void positionUpdated(const QGeoPositionInfo &info);
    void updateTimeout();
    void orientationChanged();
};

#endif // TRAFIKANTENWINDOW_H
