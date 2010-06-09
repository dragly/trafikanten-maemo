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
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->setupUi(this);

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
    this->setWindowTitle(tr("Places Nearby"));
    this->easting = easting;
    this->northing = northing;
    ui->btnSearch->hide();
    ui->txtSearch->hide();
    QString dataUrl = "http://reis.trafikanten.no/topp2009/getcloseststops.aspx?x=" + easting + "&y=" + northing + "&proposals=10"; //
    QString data = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
                   "  <soap:Body>"
                   "    <GetClosestStops xmlns=\"http://www.trafikanten.no/\">"
                   "      <c>"
                   "        <X>" + easting + "</X>"
                   "        <Y>" + northing + "</Y>"
                   "      </c>"
                   "      <proposals>unsignedByte</proposals>"
                   "    </GetClosestStops>"
                   "  </soap:Body>"
                   "</soap:Envelope>";
    qDebug() << "requesting" << dataUrl;
    QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
    manager->post(request, data);
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
    //Getting data
    QString dataUrl = "http://reis.trafikanten.no/siri/checkrealtimestop.aspx?name=" + ui->txtSearch->text(); //
    QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
    qDebug() << "requesting" << dataUrl;
    manager->get(request);
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
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
        QDomElement place = response.firstChildElement("Place");
        model->clear();
        int row = 0;
        while(!place.isNull()) {
            QDomElement placeId = place.firstChildElement("ID");
            QDomElement name = place.firstChildElement("Name");
            QStandardItem *nameItem = new QStandardItem(name.text());
            QVariantHash itemData;
            itemData.insert("placeId", placeId.text());
            itemData.insert("name", name.text());
            nameItem->setData(itemData);
            model->setItem(row, 0, nameItem);
            place = place.nextSiblingElement("Place");
            row++;
        }
        // size everything for full window scrolling
        ui->tblResults->resizeRowsToContents();
        ui->tblResults->setFixedHeight(ui->tblResults->verticalHeader()->length() + 60);
    }
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
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
