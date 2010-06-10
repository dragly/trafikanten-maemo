#ifndef COMMON_H
#define COMMON_H

#include <QtCore>
#include <QPoint>

class Departure{
//    Q_OBJECT
public:
    Departure(QString lineDestination,
            QString lineNumber,
            QString arrivalTime,
            int arrivalDifference) {
        this->lineDestination = lineDestination;
        this->lineNumber = lineNumber;
        this->arrivalTime = arrivalTime;
        this->arrivalDifference = arrivalDifference;
    }
    QString lineDestination;
    QString lineNumber;
    QString arrivalTime;
    int arrivalDifference;
};

Q_DECLARE_METATYPE(Departure *);

class Place {
//    Q_OBJECT
public:
    Place() {placeId = NULL;}
    Place(QString placeName,
            int placeId) {
        this->placeId = placeId;
        this->placeName = placeName;
    }
    QString placeName;
    int placeId;
};

Q_DECLARE_METATYPE(Place *);

class TravelStage {
//    Q_OBJECT
public:
    TravelStage() {}
    Place departureStop;
    Place arrivalStop;
    QDateTime departureTime;
    QDateTime arrivalTime;

    QString destination;
    QString lineName;
    QString transportation;
    int tourID;
};

Q_DECLARE_METATYPE(TravelStage *);

class Travel {
//    Q_OBJECT
public:
    Travel(QDateTime departureTime, QDateTime arrivalTime, QList<TravelStage*> travelStages) {
    this->arrivalTime = arrivalTime;
    this->departureTime = departureTime;
    this->travelStages = travelStages;
    }
    QList<TravelStage*> travelStages;
    QDateTime departureTime;
    QDateTime arrivalTime;
};

Q_DECLARE_METATYPE(Travel *);

#endif // COMMON_H
