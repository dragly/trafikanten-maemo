#ifndef TRAFIKANTENWINDOW_H
#define TRAFIKANTENWINDOW_H

#include <QMainWindow>

namespace Ui {
    class TrafikantenWindow;
}

class TrafikantenWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TrafikantenWindow(QWidget *parent = 0);
    ~TrafikantenWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TrafikantenWindow *ui;
};

#endif // TRAFIKANTENWINDOW_H
