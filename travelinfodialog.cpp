#include "travelinfodialog.h"
#include "ui_travelinfodialog.h"

#include <QtGui>

TravelInfoDialog::TravelInfoDialog(Travel *travel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TravelInfoDialog)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
#ifdef Q_OS_SYMBIAN
    // We need to add a back button
    QAction *backSoftKeyAction = new QAction(tr("Back"), this);
    backSoftKeyAction->setSoftKeyRole(QAction::NegativeSoftKey);
    connect(backSoftKeyAction,SIGNAL(triggered()),SLOT(close())); // when the back button is pressed, just close this window
    addAction(backSoftKeyAction);
#endif
    ui->tblResults->setItemDelegate(new TravelStageListDelegate(this, this));
    ui->tblResults->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    ui->tblResults->setModel(new TravelStageListModel(this, travel->travelStages));

    ui->tblResults->resizeRowsToContents();
    ui->tblResults->setFixedHeight(ui->tblResults->verticalHeader()->length() + 60);

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    orientationChanged(); // call this just in case we're in portrait mode from before
}

void TravelInfoDialog::orientationChanged() {
    // Change the layout of the search controls
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        _portraitMode = false;
    } else {
        _portraitMode = true;
    }
}

TravelInfoDialog::~TravelInfoDialog()
{
    delete ui;
}

void TravelInfoDialog::changeEvent(QEvent *e)
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

int TravelStageListModel::rowCount(const QModelIndex &) const {
    return travels.size();;
}

QVariant TravelStageListModel::data(const QModelIndex &index, int role) const {
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

void TravelStageListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    TravelStage *e = qVariantValue<TravelStage *>(index.data());

    QFont font = option.font;
    QRect rect = option.rect;
    QSize iconSize(48, 48);
    if(dialog->portraitMode()) {
        rect.adjust(2, 2, -10, -2);
    } else {
        rect.adjust(2, 2, -10, -5);
    }
    QPoint iconPosition = rect.topLeft();
    iconPosition.setY(iconPosition.y() + 5);

    rect.adjust(60, 0, 0, 0);

    painter->save();

    int maxLabelLength;
    if (dialog->portraitMode()) {
        maxLabelLength = 24;
    } else {
        maxLabelLength = 40;
    }

    QString lineAndDestination;
    if(e->transportation == "Walking") {
        lineAndDestination = tr("Walking");
    } else {
        QString lineDestination;
        if(e->destination.length() > maxLabelLength) {
            lineDestination = e->destination.left(maxLabelLength - 2) + "...";
        } else {
            lineDestination = e->destination;
        }
        lineAndDestination = e->lineName + " " + lineDestination;
    }
    QString departureStopPlaceName;
    if(e->departureStop.placeName.length() > maxLabelLength) {
        departureStopPlaceName = e->departureStop.placeName.left(maxLabelLength - 2) + "...";
    } else {
        departureStopPlaceName = e->departureStop.placeName;
    }
    QString arrivalStopPlaceName;
    if(e->arrivalStop.placeName.length() > maxLabelLength) {
        arrivalStopPlaceName = e->arrivalStop.placeName.left(maxLabelLength - 2) + "...";
    } else {
        arrivalStopPlaceName = e->arrivalStop.placeName;
    }

    int minutes = (int)((double)e->departureTime.secsTo(e->arrivalTime) / 60.0);

    if(dialog->portraitMode()) {

        font.setPointSizeF(font.pointSizeF() * 0.70);
        painter->setFont(font);

        painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft, lineAndDestination);
        painter->drawText(rect, Qt::AlignTop | Qt::AlignRight, tr("%n min", "", minutes));
        painter->setPen(option.palette.mid().color());
        painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, departureStopPlaceName + " " + e->departureTime.toString("hh:mm"));
        painter->drawText(rect, Qt::AlignBottom | Qt::AlignLeft, arrivalStopPlaceName + " " + e->arrivalTime.toString("hh:mm"));
    } else {
        painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft, lineAndDestination);
        painter->drawText(rect, Qt::AlignTop | Qt::AlignRight, departureStopPlaceName + " " + e->departureTime.toString("hh:mm"));

        painter->setPen(option.palette.mid().color());
        font.setPointSizeF(font.pointSizeF() * 0.70);
        painter->setFont(font);

        painter->drawText(rect, Qt::AlignBottom | Qt::AlignRight, arrivalStopPlaceName + " " + e->arrivalTime.toString("hh:mm"));
        painter->drawText(rect, Qt::AlignBottom | Qt::AlignLeft, tr("%n min", "", minutes));
    }

    if(e->transportation == "Bus" || e->transportation == "AirportBus") {
        QImage icon = QImage(":/images/bus.png").scaled(iconSize);
        painter->drawImage(iconPosition, icon);
    } else if(e->transportation == "Tram") {
        QImage icon = QImage(":/images/tram.png").scaled(iconSize);
        painter->drawImage(iconPosition, icon);
    } else if(e->transportation == "Metro") {
        QImage icon = QImage(":/images/tbane.png").scaled(iconSize);
        painter->drawImage(iconPosition, icon);
    } else if(e->transportation == "Train" || e->transportation == "AirportTrain") {
        QImage icon = QImage(":/images/train.png").scaled(iconSize);
        painter->drawImage(iconPosition, icon);
    }
    painter->restore();
}
