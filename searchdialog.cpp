#include "searchdialog.h"
#include "ui_searchdialog.h"

#include "departureswindow.h"

#include <QtGui>
#include <QtNetwork>
#include <QtXml>

SearchDialog::SearchDialog(QWidget *parent, QString easting, QString northing) :
        QDialog(parent),
        ui(new Ui::SearchDialog)
{
    Q_UNUSED(easting)
    Q_UNUSED(northing)
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->setupUi(this);
    ui->lblNoResults->hide();

    requestType = PlaceSearch;
    manager = new QNetworkAccessManager(this);
    model = new QStandardItemModel(this);
    model->setColumnCount(1);
    ui->tblResults->setModel(model);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    ui->txtSearch->setFocus();
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    orientationChanged(); // call this just in case we're in portrait mode from before
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::setNormalSearch() {
    this->setWindowTitle(tr("Search"));
    ui->btnSearch->show();
    ui->txtSearch->show();
}

void SearchDialog::searchPosition(QString easting, QString northing) {
    ui->lblNoResults->hide();
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
    this->setWindowTitle(tr("Places Nearby"));
    this->easting = easting;
    this->northing = northing;
    ui->btnSearch->hide();
    ui->txtSearch->hide();
    QString dataUrl = "http://reis.trafikanten.no/topp2009/topp2009ws.asmx";
    QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
    qDebug() << "requesting" << dataUrl;
    QString data = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
                   "  <soap:Body>"
                   "    <GetClosestStops xmlns=\"http://www.trafikanten.no/\">"
                   "      <c>"
                   "        <X>" + easting + "</X>"
                   "        <Y>" + northing + "</Y>"
                   "      </c>"
                   "      <proposals>10</proposals>"
                   "    </GetClosestStops>"
                   "  </soap:Body>"
                   "</soap:Envelope>";
    qDebug() << "request data" << data;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=\"UTF-8\"");
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.toUtf8().length());
    requestType = ClosestStops;
    manager->post(request, data.toUtf8());
}

void SearchDialog::orientationChanged() {
    // Change the layout of the search controls
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        portraitMode = false;
        ui->searchLayoutV->layout()->removeWidget(ui->btnSearch);
        ui->searchLayoutV->layout()->removeWidget(ui->txtSearch);
        ui->searchLayoutH->addWidget(ui->txtSearch);
        ui->searchLayoutH->addWidget(ui->btnSearch);
    } else {
        portraitMode = true;
        ui->searchLayoutH->removeWidget(ui->txtSearch);
        ui->searchLayoutH->removeWidget(ui->btnSearch);
        ui->searchLayoutV->layout()->addWidget(ui->txtSearch);
        ui->searchLayoutV->layout()->addWidget(ui->btnSearch);
    }

}

void SearchDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SearchDialog::on_btnSearch_clicked()
{
    ui->lblNoResults->hide();
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
    //Getting data
    QString dataUrl = "http://reis.trafikanten.no/topp2009/topp2009ws.asmx";
    QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
    QString data = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
                   "  <soap:Body>"
                   "    <GetMatches xmlns=\"http://www.trafikanten.no/\">"
                   "      <searchName>" + ui->txtSearch->text() + "</searchName>"
                   "    </GetMatches>"
                   "  </soap:Body>"
                   "</soap:Envelope>";
    qDebug() << "requesting" << dataUrl;
    qDebug() << "request data" << data;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=\"UTF-8\"");
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.toUtf8().length());
    requestType = PlaceSearch;
    manager->post(request, data.toUtf8());
}

void SearchDialog::replyFinished(QNetworkReply *reply) {
    QString data = QString::fromUtf8(reply->readAll()); // use UTF-8 encoding (why doesn't Qt detect this by itself?)
    qDebug() << "\n\n----Returned data---\n\n" << data << "\n\n\n";
    if(reply->error() == QNetworkReply::NoError) {
        QDomDocument doc;
        doc.setContent(data);
        QDomElement response = doc.documentElement();
        if(response.isNull()) {
            qDebug("No response found!");
        }
        QDomElement placeStop;
        if(requestType == ClosestStops) {
            placeStop = response.firstChildElement("soap:Body").firstChildElement("GetClosestStopsResponse").firstChildElement("GetClosestStopsResult").firstChildElement("Stop");
        } else if(requestType == PlaceSearch) {
            placeStop = response.firstChildElement("soap:Body").firstChildElement("GetMatchesResponse").firstChildElement("GetMatchesResult").firstChildElement("Place");
        }
        if(placeStop.isNull()) {
            qDebug() << "PlaceStop not found";
        }
        if(placeStop.isNull() || response.isNull()) {
            ui->lblNoResults->show();
        }
        model->clear();
        int row = 0;
        while(!placeStop.isNull()) {
            QDomElement placeId = placeStop.firstChildElement("ID");
            QDomElement name = placeStop.firstChildElement("Name");
            QDomElement type = placeStop.firstChildElement("Type");
            if(type.text() == "Stop") { // TODO: Add support for Area and POI
                QStandardItem *nameItem = new QStandardItem(name.text());
                QVariantHash itemData;
                itemData.insert("placeId", placeId.text());
                itemData.insert("name", name.text());
                nameItem->setData(itemData);
                model->setItem(row, 0, nameItem);
                row++;
            }
            if(requestType == ClosestStops) {
                placeStop = placeStop.nextSiblingElement("Stop");
            } else if (requestType == PlaceSearch) {
                placeStop = placeStop.nextSiblingElement("Place");
            }
        }
        // size everything for full window scrolling
        ui->tblResults->resizeRowsToContents();
        ui->tblResults->setFixedHeight(ui->tblResults->verticalHeader()->length() + 60);
    }
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}

void SearchDialog::on_tblResults_clicked(QModelIndex index)
{
    QStandardItem *item = model->itemFromIndex(index);
    QVariantHash itemData = item->data().toHash();
    place_ = Place(itemData.value("name").toString(), itemData.value("placeId").toString().toInt());
    setResult(QDialog::Accepted);
    hide();
}

void SearchDialog::on_txtSearch_returnPressed()
{
    on_btnSearch_clicked();
}
