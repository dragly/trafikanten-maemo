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
    explicit SearchWindow(QWidget *parent = 0);
    ~SearchWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SearchWindow *ui;
    QNetworkAccessManager *manager;
    QStandardItemModel *model;

private slots:
    void on_tblResults_clicked(QModelIndex index);
    void on_btnSearch_clicked();
    void replyFinished(QNetworkReply* reply);
};

#endif // SEARCHWINDOW_H
