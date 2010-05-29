#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QtGui>

namespace Ui {
    class SearchWindow;
}

class SearchWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SearchWindow(QWidget *parent = 0, QString easting = "", QString northing = "");
    ~SearchWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SearchWindow *ui;
    QNetworkAccessManager *manager;
    QStandardItemModel *model;
    bool portraitMode;

private slots:
    void on_txtSearch_returnPressed();
    void on_tblResults_clicked(QModelIndex index);
    void on_btnSearch_clicked();
    void replyFinished(QNetworkReply* reply);
    void orientationChanged();
};

#endif // SEARCHWINDOW_H
