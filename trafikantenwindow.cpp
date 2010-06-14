/*
 Conversion from longlat to easting/northing is (c) 2006 Jonathan Stott and GPLv3
 Rewritten using code from http://code.google.com/p/trafikanten/source/browse/src/uk/me/jstott/jcoord/LatLng.java
*/

#include <QMaemo5InformationBox>

#include "trafikantenwindow.h"
#include "ui_trafikantenwindow.h"

#include "searchdialog.h"
#include "travelsearchwindow.h"
#include "aboutdialog.h"
#include "departureswindow.h"
#include "common.h"
#include "recentwindow.h"

TrafikantenWindow::TrafikantenWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::TrafikantenWindow)
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
#endif
    ui->setupUi(this);


    search = new SearchDialog(this); // there is only one search window :)

    // Set up GPS stuff
    positionSource = QGeoPositionInfoSource::createDefaultSource(this);
    connect(positionSource, SIGNAL(positionUpdated(QGeoPositionInfo)),
            this, SLOT(positionUpdated(QGeoPositionInfo)));
    connect(positionSource, SIGNAL(updateTimeout()),
            this, SLOT(updateTimeout()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    orientationChanged(); // call this just in case we're in portrait mode from before
}

TrafikantenWindow::~TrafikantenWindow()
{
    delete ui;
}

void TrafikantenWindow::orientationChanged() {
    // Change the layout of the search controls
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        portraitMode = false;
        ui->layoutButtonsAdditional->removeWidget(ui->btnNearby);
        ui->layoutButtonsAdditional->removeWidget(ui->btnRouting);
        ui->layoutButtons->addWidget(ui->btnNearby);
        ui->layoutButtons->addWidget(ui->btnRouting);
    } else {
        portraitMode = true;
        ui->layoutButtons->removeWidget(ui->btnNearby);
        ui->layoutButtons->removeWidget(ui->btnRouting);
        ui->layoutButtonsAdditional->addWidget(ui->btnNearby);
        ui->layoutButtonsAdditional->addWidget(ui->btnRouting);
    }
}

void TrafikantenWindow::changeEvent(QEvent *e)
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

void TrafikantenWindow::on_btnSearch_clicked()
{
    positionSource->stopUpdates();
    search->setNormalSearch();
    if(portraitMode) {
        search->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
    } else {
        search->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
    }
    int result = search->exec();

    if(result == QDialog::Accepted) {
        DeparturesWindow *win = new DeparturesWindow(search->place(), this);
        if(portraitMode) {
            win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
        } else {
            win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
        }
        win->show();
    }
}

void TrafikantenWindow::on_btnNearby_clicked()
{
#ifdef Q_WS_MAEMO_5
    QMaemo5InformationBox::information(this, tr("Requesting your position using GPS/GSM"), QMaemo5InformationBox::DefaultTimeout);
#endif
    qDebug() << "Requesting update...";
    if (positionSource) {
        positionSource->setPreferredPositioningMethods(QGeoPositionInfoSource::AllPositioningMethods); // use all methods
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
        positionSource->requestUpdate(45000);
    }
}

void TrafikantenWindow::positionUpdated(const QGeoPositionInfo &info) {
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
    qDebug() << "Position updated:" << info;
    // TODO: Convert from latlong to easting and northing. See http://code.google.com/p/trafikanten/source/browse/src/uk/me/jstott/jcoord/UTMRef.java
    double UTM_F0 = 0.9996;
    double maj = 6378137.000;
    double min = 6356752.3141;
    double a = maj;
    double eSquared = ((maj * maj) - (min * min)) / (maj * maj);
    double longitude = info.coordinate().longitude();
    double latitude = info.coordinate().latitude();

    double latitudeRad = latitude * (M_PI / 180.0);
    double longitudeRad = longitude * (M_PI / 180.0);
    int longitudeZone = (int) floor((longitude + 180.0) / 6.0) + 1;

    // Special zone for Norway
    if (latitude >= 56.0 && latitude < 64.0 && longitude >= 3.0
        && longitude < 12.0) {
        longitudeZone = 32;
    }

    // Special zones for Svalbard
    if (latitude >= 72.0 && latitude < 84.0) {
        if (longitude >= 0.0 && longitude < 9.0) {
            longitudeZone = 31;
        } else if (longitude >= 9.0 && longitude < 21.0) {
            longitudeZone = 33;
        } else if (longitude >= 21.0 && longitude < 33.0) {
            longitudeZone = 35;
        } else if (longitude >= 33.0 && longitude < 42.0) {
            longitudeZone = 37;
        }
    }

    double longitudeOrigin = (longitudeZone - 1) * 6 - 180 + 3;
    double longitudeOriginRad = longitudeOrigin * (M_PI / 180.0);

    //    char UTMZone = UTMRef.getUTMLatitudeZoneLetter(latitude);

    double ePrimeSquared = (eSquared) / (1 - eSquared);

    double n =
            a
            / sqrt(1 - eSquared * sin(latitudeRad)
                   * sin(latitudeRad));
    double t = tan(latitudeRad) * tan(latitudeRad);
    double c = ePrimeSquared * cos(latitudeRad) * cos(latitudeRad);
    double A = cos(latitudeRad) * (longitudeRad - longitudeOriginRad);

    double M =
            a
            * ((1 - eSquared / 4 - 3 * eSquared * eSquared / 64 - 5 * eSquared
                * eSquared * eSquared / 256)
               * latitudeRad
               - (3 * eSquared / 8 + 3 * eSquared * eSquared / 32 + 45
                  * eSquared * eSquared * eSquared / 1024)
               * sin(2 * latitudeRad)
               + (15 * eSquared * eSquared / 256 + 45 * eSquared * eSquared
                  * eSquared / 1024) * sin(4 * latitudeRad) - (35
                                                               * eSquared * eSquared * eSquared / 3072)
               * sin(6 * latitudeRad));

    double UTMEasting =
            (UTM_F0
             * n
             * (A + (1 - t + c) * pow(A, 3.0) / 6 + (5 - 18 * t + t * t
                                                     + 72 * c - 58 * ePrimeSquared)
                * pow(A, 5.0) / 120) + 500000.0);

    double UTMNorthing =
            (UTM_F0 * (M + n
                       * tan(latitudeRad)
                       * (A * A / 2 + (5 - t + (9 * c) + (4 * c * c)) * pow(A, 4.0)
                          / 24 + (61 - (58 * t) + (t * t) + (600 * c) - (330 * ePrimeSquared))
                          * pow(A, 6.0) / 720)));


    // Adjust for the southern hemisphere
    if (latitude < 0) {
        UTMNorthing += 10000000.0;
    }
    QString northing = QString::number((int)UTMNorthing);
    QString easting = QString::number((int)UTMEasting);
    qDebug() << "northing" << northing << "easting" << easting;
    search->searchPosition(easting, northing);
    if(portraitMode) {
        search->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
    } else {
        search->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
    }
    int result = search->exec();

    if(result == QDialog::Accepted) {
        DeparturesWindow *win = new DeparturesWindow(search->place(), this);
        if(portraitMode) {
            win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
        } else {
            win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
        }
        win->show();
    }
}

void TrafikantenWindow::updateTimeout() {
    qDebug() << "Timed out";
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
    QMessageBox messageBox;
    messageBox.setWindowTitle(tr("Timed out"));
    messageBox.setText(tr("We are terribly sorry, but we're unable to locate your position at the current time. Please try again."));
    messageBox.exec();
}

void TrafikantenWindow::on_btnRouting_clicked()
{
    positionSource->stopUpdates();
    TravelSearchWindow* win = new TravelSearchWindow(this);
    if(portraitMode) {
        win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
    } else {
        win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
    }
    win->show();
}

void TrafikantenWindow::on_actionAbout_triggered()
{
    AboutDialog dialog;
    dialog.exec();
}

void TrafikantenWindow::on_btnRecent_clicked()
{
    positionSource->stopUpdates();
    RecentWindow* win = new RecentWindow(RecentWindow::Recent, this);
    if(portraitMode) {
        win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
    } else {
        win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
    }
    win->show();
}

void TrafikantenWindow::on_btnFavorites_clicked()
{
    positionSource->stopUpdates();
    RecentWindow* win = new RecentWindow(RecentWindow::Favorites, this);
    if(portraitMode) {
        win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
    } else {
        win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
    }
    win->show();
}
