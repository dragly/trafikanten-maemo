#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QtNetwork>
#include <QtGui>

#include "common.h"

namespace Ui {
    class SearchDialog;
}
class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = 0, QString easting = "", QString northing = "");
    ~SearchDialog();

    Place place() {return place_;}
    void searchPosition(QString easting = "", QString northing = "");
    void setNormalSearch();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SearchDialog *ui;
    QNetworkAccessManager *manager;
    QStandardItemModel *model;
    bool portraitMode;
    Place place_;
    QString easting;
    QString northing;

private slots:
    void on_txtSearch_returnPressed();
    void on_tblResults_clicked(QModelIndex index);
    void on_btnSearch_clicked();
    void replyFinished(QNetworkReply* reply);
    void orientationChanged();
};

#endif // SEARCHDIALOG_H
