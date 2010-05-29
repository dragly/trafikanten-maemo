#include "trafikantenwindow.h"
#include "ui_trafikantenwindow.h"

#include "searchwindow.h"

TrafikantenWindow::TrafikantenWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrafikantenWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_Maemo5StackedWindow);
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

void TrafikantenWindow::on_btnSearch_clicked()
{
    SearchWindow* win = new SearchWindow(this);
    win->show();
}
