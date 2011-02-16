#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
#ifdef Q_OS_SYMBIAN
    // We need to add a back button
    QAction *backSoftKeyAction = new QAction(tr("Back"), this);
    backSoftKeyAction->setSoftKeyRole(QAction::NegativeSoftKey);
    connect(backSoftKeyAction,SIGNAL(triggered()),SLOT(close())); // when the back button is pressed, just close this window
    addAction(backSoftKeyAction);
#endif

    QString description;
//#ifdef Q_WS_MAEMO_5
    description= tr("<h3>%9 %1</h3>Based on revision %8 and Qt %2 (%3 bit)<br/><br/>Built on %4 at %5<br /><br/>Copyright %6 %7. Released under the GNU GPL v3 license.<br/>").arg(APP_VERSION,
                 QLatin1String(QT_VERSION_STR), QString::number(QSysInfo::WordSize),
                 QLatin1String(__DATE__), QLatin1String(__TIME__), APP_YEAR,
                 APP_AUTHOR, APP_REVISION, tr("Journey Planner for Norway"));
//#endif

    QString aboutText;
//#ifdef Q_WS_MAEMO_5
    aboutText = ui->label->text().arg(description);
//#endif

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
