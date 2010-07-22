#include "travelsearchwindow.h"
#include "ui_travelsearchwindow.h"
#include "searchdialog.h"
#include "travelinfodialog.h"

#include <QtXml>

TravelSearchWindow::TravelSearchWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TravelSearchWindow)
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
#endif
    ui->setupUi(this);

    timePicker = new QMaemo5TimePickSelector(this);
    datePicker = new QMaemo5DatePickSelector(this);
    QMaemo5ValueButton *dateButton = new QMaemo5ValueButton(tr("Date"), this);
    QMaemo5ValueButton *timeButton = new QMaemo5ValueButton(tr("Time"), this);
    dateButton->setPickSelector(datePicker);
    timeButton->setPickSelector(timePicker);
    ui->dateTimeContainer->addWidget(dateButton);
    ui->dateTimeContainer->addWidget(timeButton);

    searchToDialog = new SearchDialog(this);
    searchFromDialog = new SearchDialog(this);

    manager = new QNetworkAccessManager(this);
    ui->tblResults->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->tblResults->setItemDelegate(new TravelListDelegate(this));
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    orientationChanged(); // call this just in case we're in portrait mode from before
}
void TravelSearchWindow::orientationChanged() {
    // Change the layout of the search controls
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        portraitMode = false;
    } else {
        portraitMode = true;
    }
}

void TravelSearchWindow::setPlaceFrom(Place place) {
    this->placeFrom = place;
    ui->btnPlaceFrom->setText(place.placeName);
}

void TravelSearchWindow::setPlaceTo(Place place) {
    this->placeTo = place;
    ui->btnPlaceTo->setText(place.placeName);
}

void TravelSearchWindow::setPlace(Place place, bool isFrom) {
    if(isFrom) {
        setPlaceFrom(place);
    } else {
        setPlaceTo(place);
    }
}

TravelSearchWindow::~TravelSearchWindow()
{
    delete ui;
}

void TravelSearchWindow::changeEvent(QEvent *e)
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

Place TravelSearchWindow::searchPlace(SearchDialog *search) {
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

void TravelSearchWindow::on_btnPlaceFrom_clicked()
{
    Place place = searchPlace(searchFromDialog);
    if(place.placeId != 0) {
        ui->btnPlaceFrom->setText(place.placeName);
        placeFrom = place;
    }
}

void TravelSearchWindow::on_btnPlaceTo_clicked()
{
    Place place = searchPlace(searchToDialog);
    if(place.placeId != 0) {
        ui->btnPlaceTo->setText(place.placeName);
        placeTo = place;
    }
}

void TravelSearchWindow::on_pushButton_clicked()
{
    QList<Search*> searches = Search::recent();
    Search *search = new Search();
    search->placeFrom = placeFrom;
    search->placeTo = placeTo;
    search->type = Search::Travel;
    searches.append(search);
    Search::saveRecent(searches);

    QString dataUrl = "http://reis.trafikanten.no/topp2009/topp2009ws.asmx"; //
    QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
    qDebug() << "requesting" << dataUrl;
    QString data = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">"
                   "  <soap12:Body>"
                   "    <GetTravelsAfter xmlns=\"http://www.trafikanten.no/\">"
                   "      <from>" + QString::number(placeFrom.placeId) + "</from>"
                   "      <to>" + QString::number(placeTo.placeId) + "</to>"
                   "      <departureTime>" + QDateTime(datePicker->currentDate(), timePicker->currentTime()).toString("yyyy-MM-ddThh:mm:ss") + "</departureTime>"
                   "    </GetTravelsAfter>"
                   "  </soap12:Body>"
                   "</soap12:Envelope>";
    qDebug() << "request data" << data;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=\"UTF-8\"");
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.toLatin1().length());
    manager->post(request, data.toLatin1());
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
}

void TravelSearchWindow::replyFinished(QNetworkReply *reply) {
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
        int row = 0;
        QList<Travel*> travels;
        while(!travel.isNull()) {
            QDomElement departureElement = travel.firstChildElement("DepartureTime");
            QDomElement arrivalElement = travel.firstChildElement("ArrivalTime");
            QDateTime departureTime = QDateTime::fromString(departureElement.text(), "yyyy-MM-ddThh:mm:ss");
            QDateTime arrivalTime = QDateTime::fromString(arrivalElement.text(), "yyyy-MM-ddThh:mm:ss");
            QList<TravelStage*> travelStages;
            QDomElement travelStagesElement = travel.firstChildElement("TravelStages");
            QDomElement travelStageElement = travelStagesElement.firstChildElement("TravelStage");
            while(!travelStageElement.isNull()) {
                TravelStage* travelStage = new TravelStage();
                QDomElement stageDepartureStopElement = travelStageElement.firstChildElement("DepartureStop");
                QDomElement stageDepartureStopNameElement = stageDepartureStopElement.firstChildElement("Name");
                QDomElement stageDepartureStopIDElement = stageDepartureStopElement.firstChildElement("ID");
                travelStage->departureStop = Place(stageDepartureStopNameElement.text(), stageDepartureStopIDElement.text().toInt());

                QDomElement stageArrivalStopElement = travelStageElement.firstChildElement("ArrivalStop");
                QDomElement stageArrivalStopNameElement = stageArrivalStopElement.firstChildElement("Name");
                QDomElement stageArrivalStopIDElement = stageArrivalStopElement.firstChildElement("ID");
                travelStage->arrivalStop = Place(stageArrivalStopNameElement.text(), stageArrivalStopIDElement.text().toInt());

                QDomElement stageDestinationElement = travelStageElement.firstChildElement("Destination");
                travelStage->destination = stageDestinationElement.text();
                QDomElement stageLineNameElement = travelStageElement.firstChildElement("LineName");
                travelStage->lineName = stageLineNameElement.text();
                QDomElement stageDepartureTime = travelStageElement.firstChildElement("DepartureTime");
                travelStage->departureTime = QDateTime::fromString(stageDepartureTime.text(), "yyyy-MM-ddThh:mm:ss");
                QDomElement stageArrivalTime = travelStageElement.firstChildElement("ArrivalTime");
                travelStage->arrivalTime = QDateTime::fromString(stageArrivalTime.text(), "yyyy-MM-ddThh:mm:ss");
                QDomElement stageTourIDElement = travelStageElement.firstChildElement("TourID");
                travelStage->tourID = stageTourIDElement.text().toInt();
                QDomElement stageTransportationElement = travelStageElement.firstChildElement("Transportation");
                travelStage->transportation = stageTransportationElement.text();
                travelStages.append(travelStage);

                travelStageElement = travelStageElement.nextSiblingElement("TravelStage");
            }

            Travel* travelItem = new Travel(departureTime, arrivalTime, travelStages);
            travels.append(travelItem);
            qDebug() << "departure" << departureElement.text() << departureTime.toString("hh:mm");
            travel = travel.nextSiblingElement("TravelProposal");
            row++;
        }
        QAbstractItemModel *oldModel = ui->tblResults->model();
        TravelListModel *model = new TravelListModel(this, travels);
        ui->tblResults->setModel(model);
        delete(oldModel);
        ui->tblResults->resizeRowsToContents();
        ui->tblResults->setFixedHeight(ui->tblResults->verticalHeader()->length() + 60);
    }
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
}

int TravelListModel::rowCount(const QModelIndex &) const {
    return travels.size();;
}

QVariant TravelListModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();
    if(index.row() >= rowCount() || index.row() < 0) {
        return QVariant();
    }

    if(role == Qt::DisplayRole) {
        return qVariantFromValue(travels.at(index.row()));
    }

    return QVariant();
}

void TravelListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    Travel *e = qVariantValue<Travel *>(index.data());

    QFont font = option.font;
    QRect rect = option.rect;
    rect.adjust(10, 10, -20, -7);

    painter->save();

    painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft, e->departureTime.toString("hh:mm") + " - " + e->arrivalTime.toString("hh:mm"));
    if(e->travelStages.size() > 1) {
        painter->drawText(rect, Qt::AlignTop | Qt::AlignRight, tr("%n switche(s)", "", e->travelStages.size() - 1));
    }

    painter->setPen(option.palette.mid().color());
    font.setPointSizeF(font.pointSizeF() * 0.70);
    painter->setFont(font);

    int minutes = (int)((double)e->departureTime.secsTo(e->arrivalTime) / 60.0);

    painter->drawText(rect, Qt::AlignBottom | Qt::AlignLeft, tr("%n min", "", minutes));

    painter->restore();
}

void TravelSearchWindow::on_tblResults_clicked(QModelIndex index)
{
    Travel *travel = qVariantValue<Travel *>(index.data());
    TravelInfoDialog* dialog = new TravelInfoDialog(travel, this);
    if(portraitMode) {
        dialog->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
    } else {
        dialog->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
    }
    dialog->exec();
}

void TravelSearchWindow::on_actionAddFavorite_triggered()
{
    Search *search = new Search();
    search->placeFrom = placeFrom;
    search->placeTo = placeTo;
    search->type = Search::Travel;
    Search::prependRecent(search);
}

void TravelSearchWindow::on_actionSwitch_direction_triggered()
{
    Place tmpPlace = placeFrom;
    placeFrom = placeTo;
    placeTo = tmpPlace;
    ui->btnPlaceFrom->setText(placeFrom.placeName);
    ui->btnPlaceTo->setText(placeTo.placeName);
}
