#ifndef DEPARTURES_H
#define DEPARTURES_H

#include <QMainWindow>

#include <QtNetwork>
#include <QtXml>
#include <QtGui>

namespace Ui {
    class DeparturesWindow;
}

class Departure : public QObject {
    Q_OBJECT
public:
    Departure(QString lineDestination,
            QString lineNumber,
            QString arrivalTime,
            QString arrivalDifference) : QObject() {
        this->lineDestination = lineDestination;
        this->lineNumber = lineNumber;
        this->arrivalTime = arrivalTime;
        this->arrivalDifference = arrivalDifference;
    }
    QString lineDestination;
    QString lineNumber;
    QString arrivalTime;
    QString arrivalDifference;
};

Q_DECLARE_METATYPE(Departure *)

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
    int placeId;

private slots:
    void replyFinished(QNetworkReply* reply);
    void orientationChanged();
};

class DepartureListModel : public QAbstractListModel {
    Q_OBJECT
public:
    DepartureListModel(QObject *parent = 0, QList<Departure *>list = QList<Departure *>())
        : QAbstractListModel(parent) {
        departure_list = list;
    }
    int rowCount(const QModelIndex &model = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
public slots:

private:
    QList<Departure *> departure_list;
};

class DepartureListDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    DepartureListDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};

#endif // DEPARTURES_H
