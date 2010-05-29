#include "departureswindow.h"
#include "ui_departureswindow.h"

DeparturesWindow::DeparturesWindow(int placeId, QString placeName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DeparturesWindow)
{
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    ui->setupUi(this);
    this->placeId = placeId;
    ui->lblName->setText(placeName);

    setAttribute(Qt::WA_Maemo5StackedWindow);
    manager = new QNetworkAccessManager(this);
    model = new QStandardItemModel(this);
    ui->tblResults->setModel(model);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    refreshData();
}

DeparturesWindow::~DeparturesWindow()
{
    delete ui;
}

void DeparturesWindow::changeEvent(QEvent *e)
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

void DeparturesWindow::replyFinished(QNetworkReply *reply) {
    QString data = QString::fromUtf8(reply->readAll()); // use UTF-8 encoding (why doesn't Qt detect this by itself?)
    qDebug() << "\n\n----Returned data---\n\n" << data << "\n\n\n";
    if(reply->error() == QNetworkReply::NoError) {
        QDomDocument doc;
        doc.setContent(data);
        QDomElement response = doc.documentElement();
        if(response.isNull()) {
            qDebug("No response found!");
        }
        QDomElement service = response.firstChildElement("ServiceDelivery");
        QDomElement timestamp = service.firstChildElement("ResponseTimestamp");
        QDomElement stop = service.firstChildElement("StopMonitoringDelivery");
        QDomElement visit = stop.firstChildElement("MonitoredStopVisit");
        model->clear();
        int row = 0;
        while(!visit.isNull()) {
            QDomElement destination = visit.firstChildElement("DestinationName");
            QDomElement expectedArrival = visit.firstChildElement("ExpectedArrivalTime");
            QDateTime expectedArrivalTime = QDateTime::fromString(expectedArrival.text().left(23), "yyyy-MM-ddThh:mm:ss.zzz");
            QDateTime timestampTime = QDateTime::fromString(timestamp.text().left(23), "yyyy-MM-ddThh:mm:ss.zzz");
            qDebug() << "expectedArrival" << expectedArrival.text();
            qDebug() << "times now expected" << timestampTime << expectedArrivalTime;
            int difference = timestampTime.secsTo(expectedArrivalTime);
            qDebug() << "difference" << difference;
            int diffMinutes = difference / 60;
            qDebug() << "diffMinutes" << diffMinutes;
            QString s;
            QStandardItem *destinationItem = new QStandardItem(expectedArrivalTime.toString("hh:mm") + " - " + destination.text() + " (" + QString::number(diffMinutes) + " min)");
            model->setItem(row, 0, destinationItem);
            visit = visit.nextSiblingElement("MonitoredStopVisit");
            row++;
        }
        ui->tblResults->resizeRowsToContents();
    }
}

void DeparturesWindow::refreshData() {
    //Getting data
    QString dataUrl = "http://reis.trafikanten.no/siri/sm.aspx?id=" + QString::number(placeId); //
    qDebug() << "Requesting" << dataUrl;
    QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
    manager->get(request);
}
