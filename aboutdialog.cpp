#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QString description = tr("<h3>%9 %1</h3>"
            "Based on revision %8 and Qt %2 (%3 bit)<br/>"
            "<br/>"
            "Built on %4 at %5<br />"
            "<br/>"
            "Copyright %6 %7. Released under the GNU GPL v3 license.<br/>")
            .arg(APP_VERSION,
                 QLatin1String(QT_VERSION_STR), QString::number(QSysInfo::WordSize),
                 QLatin1String(__DATE__), QLatin1String(__TIME__), APP_YEAR,
                 APP_AUTHOR, APP_REVISION, tr("Journey Planner for Norway"));

    QString aboutText = tr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
                            "p, li { white-space: pre-wrap; }"
                            "</style></head><body style=\"font-size:14pt; font-style:normal;\">"
                            "<table style=\"-qt-table-type: root; margin-top:4px; margin-bottom:4px; margin-left:4px; margin-right:4px;\">"
                            "<tr>"
                            "<td style=\"border: none;\">"
                            "%1"
                            "<p>Journey Planner for Norway gives you information about the public transport system in Akershus, Buskerud, Hedmark, Oppland, Oslo, Vestfold, Telemark og Østfold. The information is gathered from the Trafikanten API. It includes information about bus, metro, tram and boat, as well as train for all county's in Eastern Norway. The system knows where the vehicle is, and calculates the time of departure in real time.</p>"
                            "<p></p>"
                            "<p>&quot;Ca&quot; before the expected arrival time indicates that this travel lacks real time information. The travel might be delayed or arrive at a later time.</p>"
                            "<p><br />Real time data for train is delivered by Jernbaneverket.<br /><br />For more information about this application, visit <a href=\"http://www.dragly.org/projects/trafikanten-maemo/\">dragly.org/projects/trafikanten-maemo</a>.</p>"
                            "<p><br />For more information about Trafikanten and applications for other platforms, go to <a href=\"http://www.trafikanten.no/\">trafikanten.no</a>. For Trafikanten's official travel planner, go to <a href=\"http://m.trafikanten.no/\">m.trafikanten.no</a>.</p>"
                            "<h4>About the license</h4>"
                            "<p>This program is free software: you can redistribute it and/or modify</p>"
                            "<p>it under the terms of the GNU General Public License as published by</p>"
                            "<p>the Free Software Foundation, either version 3 of the License, or</p>"
                            "<p>(at your option) any later version.</p>"
                            "<p></p>"
                            "<p>This program is distributed in the hope that it will be useful,</p>"
                            "<p>but WITHOUT ANY WARRANTY; without even the implied warranty of</p>"
                            "<p>MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the</p>"
                            "<p>GNU General Public License for more details.</p>"
                            "<p></p>"
                            "<p>You should have received a copy of the GNU General Public License</p>"
                            "<p>along with this program.  If not, see &lt;http://www.gnu.org/licenses/&gt;.</p></td></tr></table></body></html>")
            .arg(description);

    ui->label->setText(aboutText);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
