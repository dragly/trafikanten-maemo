#ifndef TRAVELINFODIALOG_H
#define TRAVELINFODIALOG_H

#include <QDialog>
#include <QtGui>
#include "common.h"

namespace Ui {
    class TravelInfoDialog;
}
class TravelInfoDialog;

class TravelStageListModel : public QAbstractListModel {
    Q_OBJECT
public:
    TravelStageListModel(QObject *parent = 0, QList<TravelStage *>list = QList<TravelStage *>())
        : QAbstractListModel(parent) {
        travels = list;
    }
    int rowCount(const QModelIndex &model = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void doReset();
public slots:

private:
    QList<TravelStage *> travels;
};

class TravelStageListDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    TravelStageListDelegate(TravelInfoDialog *dialog, QObject *parent = 0)
        : QStyledItemDelegate(parent) {this->dialog = dialog;}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index);
    TravelInfoDialog* dialog;
};

class TravelInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TravelInfoDialog(Travel *travel, QWidget *parent = 0);
    ~TravelInfoDialog();
    bool portraitMode() {return _portraitMode;}

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TravelInfoDialog *ui;
    bool _portraitMode;

private slots:
    void orientationChanged();
};

#endif // TRAVELINFODIALOG_H
