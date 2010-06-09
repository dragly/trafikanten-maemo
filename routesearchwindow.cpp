#include "routesearchwindow.h"
#include "ui_routesearchwindow.h"
#include "searchdialog.h"

#include <QtXml>

RouteSearchWindow::RouteSearchWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RouteSearchWindow)
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
#endif
    ui->setupUi(this);
    manager = new QNetworkAccessManager(this);
    model = new QStandardItemModel(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    orientationChanged(); // call this just in case we're in portrait mode from before
}
void RouteSearchWindow::orientationChanged() {
    // Change the layout of the search controls
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        portraitMode = false;
    } else {
        portraitMode = true;
    }
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

Place RouteSearchWindow::searchPlace() {
    SearchDialog *search = new SearchDialog(this);
    search->setNormalSearch();
    if(portraitMode) {
        search->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
    } else {
        search->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
    }
    int result = search->exec();

    if(result == QDialog::Accepted) {
        return search->place();
    } else {
        return Place();
    }
}

void RouteSearchWindow::on_btnPlaceFrom_clicked()
{
    Place place = searchPlace();
    if(place.placeId != 0) {
        ui->btnPlaceFrom->setText(place.placeName);
        placeFrom = place;
    }
}

void RouteSearchWindow::on_btnPlaceTo_clicked()
{
    Place place = searchPlace();
    if(place.placeId != 0) {
        ui->btnPlaceTo->setText(place.placeName);
        placeTo = place;
    }
}

void RouteSearchWindow::on_pushButton_clicked()
{
    QString dataUrl = "http://reis.trafikanten.no/topp2009/topp2009ws.asmx"; //
    QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
    qDebug() << "requesting" << dataUrl;
    QString data = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">"
                   "  <soap12:Body>"
                   "    <GetTravelsAfter xmlns=\"http://www.trafikanten.no/\">"
                   "      <from>3011405</from>"
                   "      <to>3012110</to>"
                   "      <departureTime>2010-06-09T17:35:00</departureTime>"
                   "    </GetTravelsAfter>"
                   "  </soap12:Body>"
                   "</soap12:Envelope>";
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=\"UTF-8\"");
    manager->post(request, data.toLatin1());
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
}

void RouteSearchWindow::replyFinished(QNetworkReply *reply) {
    QString data = QString::fromUtf8(reply->readAll()); // use UTF-8 encoding (why doesn't Qt detect this by itself?)
    qDebug() << "\n\n----Returned data---\n\n" << data << "\n\n\n";
    if(reply->error() == QNetworkReply::NoError) {
        QDomDocument doc;
        doc.setContent(data);
        QDomElement response = doc.documentElement();
        if(response.isNull()) {
            qDebug("No response found!");
        }
        QDomElement travel = response.firstChildElement("soap:Body").firstChildElement("GetTravelsAfterResponse").firstChildElement("GetTravelsAfterResult").firstChildElement("TravelProposal");
        if(travel.isNull()) {
            qDebug() << "No travel proposals";
        }
        model->clear();
        int row = 0;
        while(!travel.isNull()) {
            qDebug() << "Found TravelProposal!";
            QDomElement placeId = travel.firstChildElement("ID");
            QDomElement name = travel.firstChildElement("Name");
            QStandardItem *nameItem = new QStandardItem(name.text());
            QVariantHash itemData;
            itemData.insert("placeId", placeId.text());
            itemData.insert("name", name.text());
            nameItem->setData(itemData);
            model->setItem(row, 0, nameItem);
            travel = travel.nextSiblingElement("Place");
            row++;
        }
        // size everything for full window scrolling
        ui->tblResults->resizeRowsToContents();
        ui->tblResults->setFixedHeight(ui->tblResults->verticalHeader()->length() + 60);
    }
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
}
