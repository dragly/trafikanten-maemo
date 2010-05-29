#include "searchwindow.h"
#include "ui_searchwindow.h"

#include "departureswindow.h"

#include <QtGui>
#include <QtNetwork>
#include <QtXml>

SearchWindow::SearchWindow(QWidget *parent, QString easting, QString northing) :
        QMainWindow(parent),
        ui(new Ui::SearchWindow)
{
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    ui->setupUi(this);
    setAttribute(Qt::WA_Maemo5StackedWindow);
    manager = new QNetworkAccessManager(this);
    model = new QStandardItemModel(this);
    model->setColumnCount(1);
    ui->tblResults->setModel(model);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    ui->txtSearch->setFocus();
    if(easting != "") {
        this->setWindowTitle("Search Nearby");
        ui->btnSearch->hide();
        ui->txtSearch->hide();
        QString dataUrl = "http://reis.trafikanten.no/topp2009/getcloseststops.aspx?x=" + easting + "&y=" + northing + "&proposals=10"; //
        qDebug() << "requesting" << dataUrl;
        QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
        manager->get(request);
    }
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    orientationChanged(); // call this just in case we're in portrait mode from before
}

SearchWindow::~SearchWindow()
{
    delete ui;
}

void SearchWindow::orientationChanged() {
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
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
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
        // size everything for full window scrolling
        ui->tblResults->resizeRowsToContents();
        ui->tblResults->setFixedHeight(ui->tblResults->verticalHeader()->length() + 60);
    }
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
}

void SearchWindow::on_tblResults_clicked(QModelIndex index)
{
    QStandardItem *item = model->itemFromIndex(index);
    QVariantHash itemData = item->data().toHash();
    int placeId = itemData.value("placeId").toString().toInt();
    QString placeName = itemData.value("name").toString();
    DeparturesWindow *win = new DeparturesWindow(placeId, placeName, this);
    if(portraitMode) {
        win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
    } else {
        win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
    }
    win->show();
}

void SearchWindow::on_txtSearch_returnPressed()
{
    on_btnSearch_clicked();
}
