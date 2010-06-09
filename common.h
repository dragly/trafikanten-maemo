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


#endif // COMMON_H
