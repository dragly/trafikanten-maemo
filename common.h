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

class Search {
public:
    enum SearchType {Realtime, Travel};
    enum ListType {Recent, Favorites};
    Place placeFrom;
    Place placeTo;
    int type;

    static QList<Search*> recent() {
        qDebug() << "loading recent";
        QSettings settings;
        QList<Search*> searches;
        int size = settings.beginReadArray("recent");
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            Search *search = new Search();
            search->placeFrom = Place(settings.value("placeFromName").toString(), settings.value("placeFromId").toInt());
            search->placeTo = Place(settings.value("placeToName").toString(), settings.value("placeToId").toInt());
            qDebug() << search->placeFrom.placeName;

            search->type = settings.value("type").toInt();

            searches.append(search);
        }
        settings.endArray();
        return searches;
    }

    static QList<Search*> favorites() {
        qDebug() << "loading favorites";
        QSettings settings;
        QList<Search*> searches;
        int size = settings.beginReadArray("favorites");
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            Search *search = new Search();
            search->placeFrom = Place(settings.value("placeFromName").toString(), settings.value("placeFromId").toInt());
            search->placeTo = Place(settings.value("placeToName").toString(), settings.value("placeToId").toInt());

            search->type = settings.value("type").toInt();

            searches.append(search);
        }
        settings.endArray();
        return searches;
    }

    static void saveFavorites(QList<Search*> searches) {
        save(searches, Favorites);
    }

    static void saveRecent(QList<Search*> searches) {
        save(searches, Recent);
    }

    static void save(QList<Search*> searches, int listType = Recent) {
        if(listType == Recent) {
            qDebug() << "saving to recent";
        } else {
            qDebug() << "saving to favorites";
        }
        QSettings settings;
        if(listType == Recent) {
            settings.beginWriteArray("recent");
        } else {
            settings.beginWriteArray("favorites");
        }
        for (int i = 0; i < searches.size(); ++i) {
            Search *search = searches.at(i);
            settings.setArrayIndex(i);

            settings.setValue("placeFromName", search->placeFrom.placeName);
            settings.setValue("placeFromId", search->placeFrom.placeId);
            settings.setValue("placeToName", search->placeTo.placeName);
            settings.setValue("placeToId", search->placeTo.placeId);

            settings.setValue("type", search->type);
        }
        settings.endArray();
    }

    static void savePrepended(Search* search, int listType = Recent) {
        QList<Search*> searches;
        if(listType == Recent) {
            searches = recent();
        } else {
            searches = favorites();
        }
        Search *alreadySearch = NULL;
        foreach(Search *oldSearch, searches) {
            if(oldSearch->type == search->type &&
               oldSearch->placeFrom.placeId == search->placeFrom.placeId &&
               oldSearch->placeTo.placeId == search->placeTo.placeId) {
                alreadySearch = oldSearch;
            }
        }
        if(alreadySearch != NULL) {
            searches.removeAll(alreadySearch);
        }
        searches.prepend(search);

        if(searches.size() > 20) {
            searches.removeLast();
        }
        if(listType == Recent) {
            saveRecent(searches);
        } else {
            saveFavorites(searches);
        }
    }

};

Q_DECLARE_METATYPE(Search *);

#endif // COMMON_H
