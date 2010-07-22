#include <QtGui/QApplication>
#include <QTranslator>
#include <QDebug>
#include <QLocale>
#include "trafikantenwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Settings
    QCoreApplication::setOrganizationName("Dragly");
    QCoreApplication::setOrganizationDomain("dragly.org");
    QCoreApplication::setApplicationName("Trafikanten");

    // Locale and translations
    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load(QString("trafikanten_") + locale);
    translator.load(QString("trafikanten_") + locale, "/usr/share/trafikanten/translations");
    qDebug() << "Loading translation" << QString("trafikanten_") << locale;
    a.installTranslator(&translator);
    TrafikantenWindow w;
#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif
    return a.exec();
}
