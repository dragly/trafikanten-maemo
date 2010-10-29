#include "travelsearchwindow.h"
#include "ui_travelsearchwindow.h"
#include "searchdialog.h"
#include "travelinfodialog.h"
#include "recentwindow.h"

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
    ui->lblNoResultsFounds->hide();

    timePicker = new QMaemo5TimePickSelector(this);
    datePicker = new QMaemo5DatePickSelector(this);
    dateButton = new QMaemo5ValueButton(tr("Date"), this);
    timeButton = new QMaemo5ValueButton(tr("Time"), this);
    dateButton->setPickSelector(datePicker);
    timeButton->setPickSelector(timePicker);
    ui->dateTimeHorizontalContainer->removeWidget(ui->btnNow);
    ui->dateTimeHorizontalContainer->addWidget(dateButton);
    ui->dateTimeHorizontalContainer->addWidget(timeButton);
    ui->dateTimeHorizontalContainer->addWidget(ui->btnNow);

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
        ui->dateTimeVerticalContainer->removeWidget(dateButton);
        ui->dateTimeHorizontalContainer->removeWidget(timeButton);
        ui->dateTimeHorizontalContainer->removeWidget(ui->btnNow);
        ui->dateTimeHorizontalContainer->addWidget(dateButton);
        ui->dateTimeHorizontalContainer->addWidget(timeButton);
        ui->dateTimeHorizontalContainer->addWidget(ui->btnNow);
    } else {
        portraitMode = true;
        ui->dateTimeVerticalContainer->removeWidget(dateButton);
        ui->dateTimeHorizontalContainer->removeWidget(ui->btnNow);
        ui->dateTimeHorizontalContainer->removeWidget(timeButton);
        ui->dateTimeVerticalContainer->insertWidget(0, dateButton);
        ui->dateTimeHorizontalContainer->addWidget(timeButton);
        ui->dateTimeHorizontalContainer->addWidget(ui->btnNow);
    }
    updateButtonText();
}

void TravelSearchWindow::setPlaceFrom(Place place) {
    this->placeFrom = place;
    updateButtonText();
}

void TravelSearchWindow::setPlaceTo(Place place) {
    this->placeTo = place;
    updateButtonText();
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
        placeFrom = place;
    }
    updateButtonText();
}

void TravelSearchWindow::on_btnPlaceTo_clicked()
{
    Place place = searchPlace(searchToDialog);
    if(place.placeId != 0) {
        placeTo = place;
    }
    updateButtonText();
}

void TravelSearchWindow::on_pushButton_clicked()
{
    ui->lblNoResultsFounds->hide();
    Search *search = new Search();
    search->placeFrom = placeFrom;
    search->placeTo = placeTo;
    search->type = Search::Travel;
    Search::savePrepended(search);

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
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.toUtf8().length());
    manager->post(request, data.toUtf8());
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
        if(travel.isNull() || response.isNull()) {
            ui->lblNoResultsFounds->show();
        } else {
            ui->lblNoResultsFounds->hide();
        }
        int row = 0;
        QList<Travel*> travels;
        while(!travel.isNull()) {
            QDomElement departureElement = travel.firstChildElement("DepartureTime");
            QDomElement arrivalElement = travel.firstChildElement("ArrivalTime");
            QDateTime departureTime = QDateTime::fromString(departureElement.text().left(19), "yyyy-MM-ddThh:mm:ss");
            QDateTime arrivalTime = QDateTime::fromString(arrivalElement.text().left(19), "yyyy-MM-ddThh:mm:ss");
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
                travelStage->departureTime = QDateTime::fromString(stageDepartureTime.text().left(19), "yyyy-MM-ddThh:mm:ss");
                QDomElement stageArrivalTime = travelStageElement.firstChildElement("ArrivalTime");
                travelStage->arrivalTime = QDateTime::fromString(stageArrivalTime.text().left(19), "yyyy-MM-ddThh:mm:ss");
                QDomElement stageTourIDElement = travelStageElement.firstChildElement("TourID");
                travelStage->tourID = stageTourIDElement.text().toInt();
                QDomElement stageTransportationElement = travelStageElement.firstChildElement("Transportation");
                travelStage->transportation = stageTransportationElement.text();
                travelStages.append(travelStage);

                travelStageElement = travelStageElement.nextSiblingElement("TravelStage");
            }
            bool swapped = true; // let's perform a bubble sort! :D
            while(swapped) {
                swapped = false;
                for(int i = 0; i < travelStages.count() - 1; i++) {
                    if(travelStages.at(i)->departureTime > travelStages.at(i + 1)->departureTime) {
                        travelStages.swap(i,i+1);
                        swapped = true;
                    }
                }
            }

            Travel* travelItem = new Travel(departureTime, arrivalTime, travelStages);
            travels.append(travelItem);
            qDebug() << "departure" << departureElement.text() << departureTime.toString("hh:mm");
            travel = travel.nextSiblingElement("TravelProposal");
            row++;
        }
        bool swappedTravels = true; // let's perform a bubble sort! :D
        while(swappedTravels) {
            swappedTravels = false;
            for(int i = 0; i < travels.count() - 1; i++) {
                if(travels.at(i)->departureTime > travels.at(i + 1)->departureTime) {
                    travels.swap(i,i+1);
                    swappedTravels = true;
                }
            }
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
    return travels.size();
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
    if(e->travelStagesNum > 1) {
        painter->drawText(rect, Qt::AlignTop | Qt::AlignRight, tr("%n switche(s)", "", e->travelStagesNum - 1));
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
    Search::savePrepended(search, Search::Favorites);
}

void TravelSearchWindow::updateButtonText() {
    int maxLabelLength;
    if (portraitMode) {
        maxLabelLength = 24;
    } else {
        maxLabelLength = 40;
    }
    QString placeFromText;
    if(placeFrom.placeName.length() > maxLabelLength) {
        placeFromText = placeFrom.placeName.left(maxLabelLength - 2) + "...";
    } else {
        placeFromText = placeFrom.placeName;
    }
    QString placeToText;
    if(placeTo.placeName.length() > maxLabelLength) {
        placeToText = placeTo.placeName.left(maxLabelLength - 2) + "...";
    } else {
        placeToText = placeTo.placeName;
    }
    if(placeFromText.isNull()) {
        ui->btnPlaceFrom->setText(tr("Select Departure"));
    } else {
        ui->btnPlaceFrom->setText(placeFromText);
    }
    if(placeToText.isNull()) {
        ui->btnPlaceTo->setText(tr("Select Destination"));
    } else {
        ui->btnPlaceTo->setText(placeToText);
    }
}

void TravelSearchWindow::on_actionSwitch_direction_triggered()
{
    Place tmpPlace = placeFrom;
    placeFrom = placeTo;
    placeTo = tmpPlace;

    updateButtonText();
}

void TravelSearchWindow::favoritePlaceSelected(Place place) {
    if(favoriteSelectMode == FavoriteFrom) {
        placeFrom = place;
    } else if(favoriteSelectMode == FavoriteTo) {
        placeTo = place;
    }
    updateButtonText();
}

void TravelSearchWindow::showFavoriteMessage(Place place, int mode) {

    bool listFavorites = false;
    Search *search = new Search();
    search->placeFrom = place;
    search->placeTo = Place();
    search->type = Search::Realtime;
    bool contains = Search::contains(Search::favoritesRealtime(), search);
    if(!place.isNull() && !contains) {
        QMessageBox msg(this);
        msg.setText(tr("Do you wish to save the current selection as a favorite or select "
                       "a different place from your list of favorites?"));
        msg.setWindowTitle(tr("Save selection or find favorite?"));
        QPushButton *saveButton = msg.addButton("Save as favorite", QMessageBox::YesRole);
        QPushButton *selectButton = msg.addButton("Select favorite", QMessageBox::YesRole);
        msg.setStandardButtons(QMessageBox::Cancel);
        msg.exec();
        if(msg.clickedButton() == saveButton) {
            Search::savePrepended(search, Search::Favorites);
        } else if(msg.clickedButton() == selectButton) {
            listFavorites = true;
        }
    }

    if(place.isNull() || contains || listFavorites){
        favoriteSelectMode = mode;
        RecentWindow* win = new RecentWindow(RecentWindow::FavoritesRealtime, this);
        connect(win, SIGNAL(placeSelected(Place)), SLOT(favoritePlaceSelected(Place)));
        if(portraitMode) {
            win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
        } else {
            win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
        }
        win->show();
    }
}

void TravelSearchWindow::on_btnFromFavorite_clicked()
{
    showFavoriteMessage(placeFrom, FavoriteFrom);
}

void TravelSearchWindow::on_btnToFavorite_clicked()
{
    showFavoriteMessage(placeTo, FavoriteTo);
}

void TravelSearchWindow::on_btnNow_clicked()
{
    datePicker->setCurrentDate(QDate::currentDate());
    timePicker->setCurrentTime(QTime::currentTime());
}
