#ifndef DEPARTURES_H
#define DEPARTURES_H

#include <QMainWindow>

#include <QtNetwork>
#include <QtXml>
#include <QtGui>

namespace Ui {
    class DeparturesWindow;
}

class DeparturesWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeparturesWindow(int placeId, QString placeName, QWidget *parent = 0);
    ~DeparturesWindow();

    void refreshData();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DeparturesWindow *ui;
    QNetworkAccessManager *manager;
    QStandardItemModel *model;
    int placeId;

private slots:
    void replyFinished(QNetworkReply* reply);
};

#endif // DEPARTURES_H
