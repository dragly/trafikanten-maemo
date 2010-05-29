#include "trafikantenwindow.h"
#include "ui_trafikantenwindow.h"

TrafikantenWindow::TrafikantenWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrafikantenWindow)
{
    ui->setupUi(this);
}

TrafikantenWindow::~TrafikantenWindow()
{
    delete ui;
}

void TrafikantenWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
