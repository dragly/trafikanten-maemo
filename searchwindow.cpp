#include "searchwindow.h"
#include "ui_searchwindow.h"

#include "departureswindow.h"

#include <QtGui>
#include <QtNetwork>
#include <QtXml>

SearchWindow::SearchWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SearchWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_Maemo5StackedWindow);
    manager = new QNetworkAccessManager(this);
    model = new QStandardItemModel(this);
    model->setColumnCount(1);
    ui->tblResults->setModel(model);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
}

SearchWindow::~SearchWindow()
{
    delete ui;
}

void SearchWindow::changeEvent(QEvent *e)
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

void SearchWindow::on_btnSearch_clicked()
{
    //Getting data
    QString dataUrl = "http://reis.trafikanten.no/siri/checkrealtimestop.aspx?name=" + ui->txtSearch->text(); //
    QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
    manager->get(request);
}

void SearchWindow::replyFinished(QNetworkReply *reply) {
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
    }
}

void SearchWindow::on_tblResults_clicked(QModelIndex index)
{
    QStandardItem *item = model->itemFromIndex(index);
    QVariantHash itemData = item->data().toHash();
    int placeId = itemData.value("placeId").toString().toInt();
    QString placeName = itemData.value("name").toString();
    DeparturesWindow *win = new DeparturesWindow(placeId, placeName, this);
    win->show();
}
