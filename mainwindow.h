#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "readmaus.h"
#include <QString>
#include <QPen>
#include <QFont>
#include "settings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void next_spill();
    void previous_spill();
    void next_event();
    void previous_event();
    void choose_event();
    void choose_spill();
    void choose_open_file();
    void open_settings();

private:
    Ui::MainWindow *ui;
    Settings* settings_window;
    ReadMAUS* read_data;

    void setup_ui();

    QString filename;
    int spillNumber, eventNumber;
    QString spillLabel, eventLabel;

    void getData(int start_spill);
    void replot();



    QHash<int, QHash<int, QVector<QVector<double> > > > data;
    QHash<int, QVector<QVector<double> > > spill;
    QVector<QVector<double> > event;


    void read_settings();
    void plot_settings();
    void position_plots();
    void momentum_plots();
    void time_plots();



};

#endif // MAINWINDOW_H
