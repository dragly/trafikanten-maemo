#include "departureswindow.h"
#include "ui_departureswindow.h"

#ifdef Q_WS_MAEMO_5
#include <QMaemo5InformationBox>
#endif
#include "travelsearchwindow.h"

DeparturesWindow::DeparturesWindow(Place place, QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::DeparturesWindow)
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->setupUi(this);
#ifdef Q_OS_SYMBIAN
    // First of all, the font is too big in Symbian
    QFont newFont = ui->lblName->font();
    newFont.setPointSize(10);
    ui->lblName->setFont(newFont);
    // We need to add a back button
    QAction *backSoftKeyAction = new QAction(tr("Back"), this);
    backSoftKeyAction->setSoftKeyRole(QAction::NegativeSoftKey);
    connect(backSoftKeyAction,SIGNAL(triggered()),SLOT(close())); // when the back button is pressed, just close this window
    addAction(backSoftKeyAction);
#endif
    this->place = place;
    ui->lblNoDepartures->hide();
    Search *search = new Search();
    search->placeFrom = place;
    search->type = Search::Realtime;
    Search::savePrepended(search);

    delegate = new DepartureListDelegate(this);
    ui->tblResults->setItemDelegate(delegate);
    ui->tblResults->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    refreshData();
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    orientationChanged();
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

void DeparturesWindow::orientationChanged() {
    //    qDebug() << "table size width" << ui->tblResults->sizeHint().width();
    //    ui->tblResults->resizeColumnsToContents();
    //    ui->tblResults->resizeRowsToContents();
    //    qDebug() << "updated table size width" << ui->tblResults->sizeHint().width();
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int maxLabelLength;
    if (screenGeometry.width() > screenGeometry.height()) {
        _portraitMode = false;
        maxLabelLength = 43;
    } else {
        _portraitMode = true;
        maxLabelLength = 24;
    }
    QString labelText;
    if(place.placeName.length() > maxLabelLength) {
        labelText = place.placeName.left(maxLabelLength - 2) + "...";
    } else {
        labelText = place.placeName;
    }
    ui->lblName->setText(labelText);
}

void DeparturesWindow::replyFinished(QNetworkReply *reply) {
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
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
        int row = 0;
        QList<Departure*> departures;

        if(visit.isNull() || response.isNull()) {
            qDebug("No visits found!");
            ui->lblNoDepartures->show();
        }
        while(!visit.isNull()) {
            QDomElement destination = visit.firstChildElement("DestinationName");
            QDomElement line = visit.firstChildElement("LineRef");
            QDomElement expectedArrival = visit.firstChildElement("ExpectedArrivalTime");
            QDateTime expectedArrivalTime = QDateTime::fromString(expectedArrival.text().left(19), "yyyy-MM-ddThh:mm:ss");
            QDateTime timestampTime = QDateTime::fromString(timestamp.text().left(19), "yyyy-MM-ddThh:mm:ss");
            qDebug() << "expectedArrival" << expectedArrival.text();
            qDebug() << "times now expected" << timestampTime << expectedArrivalTime;
            int difference = timestampTime.secsTo(expectedArrivalTime);
            qDebug() << "difference" << difference;
            difference += 15; // Trafikanten uses a 15 second delay to show their times
            int diffMinutes = difference / 60;
            Departure *departure = new Departure(destination.text(), line.text(), expectedArrivalTime.toString("hh:mm"), diffMinutes);
            departures.append(departure);
            visit = visit.nextSiblingElement("MonitoredStopVisit");
            row++;
        }
        bool swapped = true; // let's perform a bubble sort! :D
        while(swapped) {
            swapped = false;
            for(int i = 0; i < departures.count() - 1; i++) {
                if(departures.at(i)->arrivalDifference > departures.at(i + 1)->arrivalDifference) {
                    departures.swap(i,i+1);
                    swapped = true;
                }
            }
        }

        QAbstractItemModel *oldModel = ui->tblResults->model();
        DepartureListModel *model = new DepartureListModel(this, departures);
        ui->tblResults->setModel(model);
        delete(oldModel);
        ui->tblResults->resizeRowsToContents();
        ui->tblResults->setFixedHeight(ui->tblResults->verticalHeader()->length() + 60);
    }
}

void DeparturesWindow::refreshData() {
    //Getting data
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
    ui->lblNoDepartures->hide();
    QString dataUrl = "http://reis.trafikanten.no/siri/sm.aspx?id=" + QString::number(place.placeId); //
    qDebug() << "Requesting" << dataUrl;
    QNetworkRequest request = QNetworkRequest(QUrl(dataUrl));
    manager->get(request);
}

void DeparturesWindow::on_actionRoute_from_triggered()
{
    TravelSearchWindow* win = new TravelSearchWindow(this);
    win->setPlace(place, true);
    if(_portraitMode) {
#ifdef Q_WS_MAEMO_5
        win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
#endif
    } else {
#ifdef Q_WS_MAEMO_5
        win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
#endif
    }
#if defined(Q_WS_S60)
    win->showMaximized();
#else
    win->show();
#endif
}

void DeparturesWindow::on_actionRoute_to_triggered()
{
    TravelSearchWindow* win = new TravelSearchWindow(this);
    win->setPlace(place, false);
    if(_portraitMode) {
#ifdef Q_WS_MAEMO_5
        win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
#endif
    } else {
#ifdef Q_WS_MAEMO_5
        win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
#endif
    }
#if defined(Q_WS_S60)
    win->showMaximized();
#else
    win->show();
#endif
}

void DepartureListModel::doReset() {
    reset();
}

int DepartureListModel::rowCount(const QModelIndex &) const {
    return departure_list.size();;
}

QVariant DepartureListModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();
    if(index.row() >= rowCount() || index.row() < 0) {
        return QVariant();
    }

    if(role == Qt::DisplayRole) {
        return qVariantFromValue(departure_list.at(index.row()));
    }

    return QVariant();
}

void DepartureListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    Departure *e = qVariantValue<Departure *>(index.data());

    QFont font = option.font;
    QRect rect = option.rect;
    rect.adjust(10, 10, -20, -7);

    int maxLabelLength = 40;
    if (departuresWindow->portraitMode()) {
        maxLabelLength = 24;
    }
    QString lineDestination;
    if(e->lineDestination.length() > maxLabelLength) {
        lineDestination = e->lineDestination.left(maxLabelLength - 2) + "...";
    } else {
        lineDestination = e->lineDestination;
    }

    painter->save();

    painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft, e->lineNumber + " " + lineDestination);

    QString timeText;
    if(e->arrivalDifference < 1) {
        timeText = tr("now");
    } else {
        timeText = QString::number(e->arrivalDifference) + tr(" min");
    }
    painter->drawText(rect, Qt::AlignTop | Qt::AlignRight, timeText);

    painter->setPen(option.palette.mid().color());
    font.setPointSizeF(font.pointSizeF() * 0.70);
    painter->setFont(font);

    painter->drawText(rect, Qt::AlignBottom | Qt::AlignLeft, e->arrivalTime);

    painter->restore();
}

QSize DepartureListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) {
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize();
}

void DeparturesWindow::on_actionAddFavorite_triggered()
{
    Search *search = new Search();
    search->placeFrom = place;
    search->type = Search::Realtime;
    Search::savePrepended(search, Search::Favorites);
}

void DeparturesWindow::on_actionRefresh_triggered()
{
    refreshData();
}

void DeparturesWindow::on_refreshButton_clicked()
{
    refreshData();
}
