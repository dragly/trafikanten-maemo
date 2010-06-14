#include "recentwindow.h"
#include "ui_recentwindow.h"

#include "departureswindow.h"
#include "travelsearchwindow.h"
#include "common.h"

RecentWindow::RecentWindow(Mode mode, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RecentWindow)
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->setupUi(this);

    this->mode = mode;
    QList<Search*> searches;
    if(mode == Recent) {
        setWindowTitle(tr("Recent searches"));
        searches = Search::recent();
    } else {
        setWindowTitle(tr("Favorite searches"));
        searches = Search::favorites();
    }

    SearchListModel *model = new SearchListModel(this, searches);
    ui->tblResults->setModel(model);
    ui->tblResults->setItemDelegate(new SearchListDelegate(this));
    ui->tblResults->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->tblResults->resizeRowsToContents();
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    orientationChanged(); // call this just in case we're in portrait mode from before
}

void RecentWindow::orientationChanged() {
    // Change the layout of the search controls
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        portraitMode = false;
    } else {
        portraitMode = true;
    }

}

RecentWindow::~RecentWindow()
{
    delete ui;
}

void RecentWindow::changeEvent(QEvent *e)
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
int SearchListModel::rowCount(const QModelIndex &) const {
    return searches.size();;
}

QVariant SearchListModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();
    if(index.row() >= rowCount() || index.row() < 0) {
        return QVariant();
    }

    if(role == Qt::DisplayRole) {
        return qVariantFromValue(searches.at(index.row()));
    }

    return QVariant();
}

void SearchListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    Search *e = qVariantValue<Search *>(index.data());

    QFont font = option.font;
    QRect rect = option.rect;
    rect.adjust(10, 10, -20, -7);

    painter->save();

    if(e->type == Search::Realtime) {
        painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft, e->placeFrom.placeName);
        painter->drawText(rect, Qt::AlignTop | Qt::AlignRight, tr("Realtime"));
    } else {
        painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft, e->placeFrom.placeName + " -- " + e->placeTo.placeName);
        painter->drawText(rect, Qt::AlignTop | Qt::AlignRight, tr("Travel"));
    }

    painter->restore();
}

void RecentWindow::on_tblResults_clicked(QModelIndex index)
{
    Search *search = qVariantValue<Search *>(index.data());

    if(search->type == Search::Realtime) {
        DeparturesWindow *win = new DeparturesWindow(search->placeFrom, this);
        if(portraitMode) {
            win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
        } else {
            win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
        }
        win->show();
    } else if(search->type == Search::Travel) {
        TravelSearchWindow *win = new TravelSearchWindow(this);
        win->setPlaceFrom(search->placeFrom);
        win->setPlaceTo(search->placeTo);
        if(portraitMode) {
            win->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
        } else {
            win->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
        }
        win->show();
    }
}
