#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setup_ui();
    filename = "~/work/Software/ReadMAUSOutput/InputFiles/07157_offline/07157_recon.root";
    //getData();
    //replot();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup_ui(){
    spillNumber = 0;
    eventNumber = 0;

    connect(ui->btn_nextEvent, SIGNAL(clicked()), SLOT(next_event()));
    connect(ui->btn_nextSpill, SIGNAL(clicked()), SLOT(next_spill()));
    connect(ui->int_goToSpill, SIGNAL(valueChanged(int)), SLOT(choose_spill()));

    connect(ui->btn_previousEvent, SIGNAL(clicked()), SLOT(previous_event()));
    connect(ui->btn_previousSpill, SIGNAL(clicked()), SLOT(previous_spill()));
    connect(ui->int_goToEvent, SIGNAL(valueChanged(int)), SLOT(choose_event()));

    connect(ui->btn_inputFile, SIGNAL(clicked()), SLOT(choose_open_file()));

    connect(ui->btn_settings, SIGNAL(clicked()), SLOT(open_settings()));


    settings_window = new Settings();
    read_data = new ReadMAUS();
    read_settings();
    plot_settings();
}

void MainWindow::open_settings(){
    settings_window->exec();
    read_settings();
}

void MainWindow::read_settings(){
    QVector<double> tof0_location = settings_window->GetTOF0Settings();
    QVector<double> tof1_location = settings_window->GetTOF1Settings();
    QVector<double> tku_location = settings_window->GetTKUSettings();
    QVector<double> tkd_location = settings_window->GetTKDSettings();
    QVector<double> tof2_location = settings_window->GetTOF2Settings();
    int spillRange = settings_window->GetSpillRange();

    read_data->SetDetectorPositions(tof0_location, tof1_location, tku_location, tkd_location, tof2_location);
    read_data->SetSpillRange(spillRange);
}


void MainWindow::choose_open_file(){
    QStringList filenames;
    QFileDialog dialog(this);
    dialog.setDirectory(ui->line_inputFile->text());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setNameFilter(tr("ROOT Files (*.root)"));
    if(dialog.exec()){
        filenames = dialog.selectedFiles();
    }

    if(!filenames.empty()){
        ui->line_inputFile->setText(filenames.first());
        filename = filenames.first();
        data.clear();
        spill.clear();
        event.clear();
        getData(0);
        replot();
    }

}

void MainWindow::plot_settings(){
    position_plots();
    momentum_plots();
    time_plots();
}

void MainWindow::position_plots(){
    QPen pen;

    /*
     * (z, x) and (z, y) plot settings:
     * -> colours, symbols, plot interactivity
     */

    ui->plot_position_xz->addGraph(); // graph 0
    ui->plot_position_xz->addGraph(); // tof0, graph 1
    ui->plot_position_xz->addGraph(); // tof1, graph 2
    ui->plot_position_xz->addGraph(); // tku, graph 3
    ui->plot_position_xz->addGraph(); // tkd, graph 4
    ui->plot_position_xz->addGraph(); // tof2, graph 5

    int yBoundLower = -180.0;
    int yBoundUpper = 180.0;

    ui->plot_position_xz->xAxis->setLabel("z (mm)");
    ui->plot_position_xz->yAxis->setLabel("x (mm)");
    ui->plot_position_xz->xAxis->setRange(5000.0, 25000.0);
    ui->plot_position_xz->yAxis->setRange(yBoundLower, yBoundUpper);

    pen.setColor(Qt::gray);
    ui->plot_position_xz->graph(0)->setPen(pen);

    pen.setColor(Qt::darkBlue);
    ui->plot_position_xz->graph(1)->setPen(pen);
    ui->plot_position_xz->graph(1)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_xz->graph(1)->setLineStyle(QCPGraph::lsNone);

    pen.setColor(Qt::red);
    ui->plot_position_xz->graph(2)->setPen(pen);
    ui->plot_position_xz->graph(2)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_xz->graph(2)->setLineStyle(QCPGraph::lsNone);

    pen.setColor(Qt::darkGreen);
    ui->plot_position_xz->graph(3)->setPen(pen);
    ui->plot_position_xz->graph(3)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_xz->graph(3)->setLineStyle(QCPGraph::lsNone);

    pen.setColor(Qt::green);
    ui->plot_position_xz->graph(4)->setPen(pen);
    ui->plot_position_xz->graph(4)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_xz->graph(4)->setLineStyle(QCPGraph::lsNone);

    pen.setColor(Qt::cyan);
    ui->plot_position_xz->graph(5)->setPen(pen);
    ui->plot_position_xz->graph(5)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_xz->graph(5)->setLineStyle(QCPGraph::lsNone);


    // interactive elements set here
    ui->plot_position_xz->setInteraction(QCP::iRangeDrag, true);
    ui->plot_position_xz->setInteraction(QCP::iRangeZoom, true);

    // legend set here
    ui->plot_position_xz->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    ui->plot_position_xz->legend->setVisible(true);
    ui->plot_position_xz->graph(0)->removeFromLegend();
    ui->plot_position_xz->graph(1)->setName("TOF0");
    ui->plot_position_xz->graph(2)->setName("TOF1");
    ui->plot_position_xz->graph(3)->setName("TkUS");
    ui->plot_position_xz->graph(4)->setName("TkDS");
    ui->plot_position_xz->graph(5)->setName("TOF2");



    ui->plot_position_yz->addGraph(); // graph 0
    ui->plot_position_yz->addGraph(); // tof0, graph 1
    ui->plot_position_yz->addGraph(); // tof1, graph 2
    ui->plot_position_yz->addGraph(); // tku, graph 3
    ui->plot_position_yz->addGraph(); // tkd, graph 4
    ui->plot_position_yz->addGraph(); // tof2, graph 5

    ui->plot_position_yz->xAxis->setLabel("z (mm)");
    ui->plot_position_yz->yAxis->setLabel("y (mm)");
    ui->plot_position_yz->xAxis->setRange(5000.0, 25000.0);
    ui->plot_position_yz->yAxis->setRange(yBoundLower, yBoundUpper);

    pen.setColor(Qt::gray);
    ui->plot_position_yz->graph(0)->setPen(pen);

    pen.setColor(Qt::darkBlue);
    ui->plot_position_yz->graph(1)->setPen(pen);
    ui->plot_position_yz->graph(1)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_yz->graph(1)->setLineStyle(QCPGraph::lsNone);

    pen.setColor(Qt::red);
    ui->plot_position_yz->graph(2)->setPen(pen);
    ui->plot_position_yz->graph(2)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_yz->graph(2)->setLineStyle(QCPGraph::lsNone);

    pen.setColor(Qt::darkGreen);
    ui->plot_position_yz->graph(3)->setPen(pen);
    ui->plot_position_yz->graph(3)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_yz->graph(3)->setLineStyle(QCPGraph::lsNone);

    pen.setColor(Qt::green);
    ui->plot_position_yz->graph(4)->setPen(pen);
    ui->plot_position_yz->graph(4)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_yz->graph(4)->setLineStyle(QCPGraph::lsNone);

    pen.setColor(Qt::cyan);
    ui->plot_position_yz->graph(5)->setPen(pen);
    ui->plot_position_yz->graph(5)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_position_yz->graph(5)->setLineStyle(QCPGraph::lsNone);

    // interactive elements set here
    ui->plot_position_yz->setInteraction(QCP::iRangeDrag, true);
    ui->plot_position_yz->setInteraction(QCP::iRangeZoom, true);

    // legend set here
    ui->plot_position_yz->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    ui->plot_position_yz->legend->setVisible(true);
    ui->plot_position_yz->graph(0)->removeFromLegend();
    ui->plot_position_yz->graph(1)->setName("TOF0");
    ui->plot_position_yz->graph(2)->setName("TOF1");
    ui->plot_position_yz->graph(3)->setName("TkUS");
    ui->plot_position_yz->graph(4)->setName("TkDS");
    ui->plot_position_yz->graph(5)->setName("TOF2");


}

void MainWindow::time_plots(){

    QPen pen;
    pen.setColor(Qt::gray);

    ui->plot_tof0_to_tof1->addGraph(); // time of flight histogram
    ui->plot_tof0_to_tof1->addGraph(); // time of flight of this particle
    ui->plot_tof0_to_tof1->xAxis->setLabel("Time of flight: TOF0 to TOF1 (ns)");
    ui->plot_tof0_to_tof1->yAxis->setLabel("Number of Particles");
    ui->plot_tof0_to_tof1->setInteraction(QCP::iRangeDrag, true);
    ui->plot_tof0_to_tof1->setInteraction(QCP::iRangeZoom, true);
    ui->plot_tof0_to_tof1->graph(0)->setPen(pen);


    ui->plot_tof0_to_tof2->addGraph(); // time of flight histogram
    ui->plot_tof0_to_tof2->addGraph(); // time of flight of this particle
    ui->plot_tof0_to_tof2->xAxis->setLabel("Time of flight: TOF0 to TOF2 (ns)");
    ui->plot_tof0_to_tof2->yAxis->setLabel("Number of Particles");
    ui->plot_tof0_to_tof2->setInteraction(QCP::iRangeDrag, true);
    ui->plot_tof0_to_tof2->setInteraction(QCP::iRangeZoom, true);
    ui->plot_tof0_to_tof2->graph(0)->setPen(pen);


    ui->plot_tof1_to_tof2->addGraph(); // time of flight histogram
    ui->plot_tof1_to_tof2->addGraph(); // time of flight of this particle
    ui->plot_tof1_to_tof2->xAxis->setLabel("Time of flight: TOF1 to TOF2 (ns)");
    ui->plot_tof1_to_tof2->yAxis->setLabel("Number of Particles");
    ui->plot_tof1_to_tof2->setInteraction(QCP::iRangeDrag, true);
    ui->plot_tof1_to_tof2->setInteraction(QCP::iRangeZoom, true);
    ui->plot_tof1_to_tof2->graph(0)->setPen(pen);

    pen.setColor(Qt::red);
    ui->plot_tof0_to_tof1->graph(1)->setPen(pen);
    ui->plot_tof0_to_tof1->graph(1)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_tof0_to_tof1->graph(1)->setLineStyle(QCPGraph::lsNone);

    ui->plot_tof0_to_tof2->graph(1)->setPen(pen);
    ui->plot_tof0_to_tof2->graph(1)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_tof0_to_tof2->graph(1)->setLineStyle(QCPGraph::lsNone);

    ui->plot_tof1_to_tof2->graph(1)->setPen(pen);
    ui->plot_tof1_to_tof2->graph(1)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_tof1_to_tof2->graph(1)->setLineStyle(QCPGraph::lsNone);

    ui->plot_tof0_to_tof1->legend->visible();
    ui->plot_tof0_to_tof1->graph(0)->setName("All events");
    ui->plot_tof0_to_tof1->graph(1)->setName("This event");

    ui->plot_tof0_to_tof2->legend->visible();
    ui->plot_tof0_to_tof2->graph(0)->setName("All events");
    ui->plot_tof0_to_tof2->graph(1)->setName("This event");

    ui->plot_tof1_to_tof2->legend->visible();
    ui->plot_tof1_to_tof2->graph(0)->setName("All events");
    ui->plot_tof1_to_tof2->graph(1)->setName("This event");
}

void MainWindow::momentum_plots(){

    ui->plot_momentum_t->addGraph(); // graph 0, all Px
    ui->plot_momentum_t->addGraph(); // graph 1, upstream tracker Px
    ui->plot_momentum_t->addGraph(); // graph 2, downstream tracker Px

    ui->plot_momentum_t->addGraph(); // graph 3, all Py
    ui->plot_momentum_t->addGraph(); // graph 4, upstream tracker Py
    ui->plot_momentum_t->addGraph(); // graph 5, downstream tracker Py

    ui->plot_momentum_t->xAxis->setLabel("z (mm)");
    ui->plot_momentum_t->xAxis->setRange(5000.0, 25000.0);
    ui->plot_momentum_t->yAxis->setLabel("Px or Py (MeV)");
    ui->plot_momentum_t->yAxis->setRange(-50.0, 50.0);

    ui->plot_momentum_t->setInteraction(QCP::iRangeDrag, true);
    ui->plot_momentum_t->setInteraction(QCP::iRangeZoom, true);

    ui->plot_momentum_z->addGraph(); // graph 0, all Pz
    ui->plot_momentum_z->addGraph(); // graph 1, upstream tracker Pz
    ui->plot_momentum_z->addGraph(); // graoh 2, downstream tracker Pz

    ui->plot_momentum_z->xAxis->setLabel("z (mm)");
    ui->plot_momentum_z->xAxis->setRange(5000.0, 25000.0);
    ui->plot_momentum_z->yAxis->setLabel("Pz (MeV)");
    ui->plot_momentum_z->yAxis->setRange(100.0, 400.0);

    ui->plot_momentum_z->setInteraction(QCP::iRangeDrag, true);
    ui->plot_momentum_z->setInteraction(QCP::iRangeZoom, true);

    QPen pen;
    pen.setColor(Qt::darkBlue);
    ui->plot_momentum_t->graph(0)->setPen(pen);
    ui->plot_momentum_t->graph(0)->setName("Px");
    pen.setColor(Qt::darkGreen);
    ui->plot_momentum_t->graph(1)->setPen(pen);
    ui->plot_momentum_t->graph(1)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_momentum_t->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui->plot_momentum_t->graph(1)->setName("Px, Upstream Tracker");
    pen.setColor(Qt::green);
    ui->plot_momentum_t->graph(2)->setPen(pen);
    ui->plot_momentum_t->graph(2)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_momentum_t->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->plot_momentum_t->graph(2)->setName("Px, Downstream Tracker");


    pen.setColor(Qt::darkRed);
    ui->plot_momentum_t->graph(3)->setPen(pen);
    ui->plot_momentum_t->graph(3)->setName("Py");
    pen.setColor(Qt::darkGreen);
    ui->plot_momentum_t->graph(4)->setPen(pen);
    ui->plot_momentum_t->graph(4)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_momentum_t->graph(4)->setLineStyle(QCPGraph::lsNone);
    ui->plot_momentum_t->graph(4)->setName("Py, Upstream Tracker");
    pen.setColor(Qt::green);
    ui->plot_momentum_t->graph(5)->setPen(pen);
    ui->plot_momentum_t->graph(5)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_momentum_t->graph(5)->setLineStyle(QCPGraph::lsNone);
    ui->plot_momentum_t->graph(5)->setName("Py, Downstream Tracker");
    ui->plot_momentum_t->legend->setVisible(true);

    pen.setColor(Qt::darkBlue);
    ui->plot_momentum_z->graph(0)->setPen(pen);
    pen.setColor(Qt::darkGreen);
    ui->plot_momentum_z->graph(1)->setPen(pen);
    ui->plot_momentum_z->graph(1)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_momentum_z->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui->plot_momentum_z->graph(1)->setName("Pz, Upstream Tracker");
    pen.setColor(Qt::green);
    ui->plot_momentum_z->graph(2)->setPen(pen);
    ui->plot_momentum_z->graph(1)->setName("Pz, Downstream Tracker");
    ui->plot_momentum_z->graph(2)->setScatterStyle(QCPScatterStyle::ssSquare);
    ui->plot_momentum_z->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->plot_momentum_z->legend->setVisible(true);


}


void MainWindow::next_event(){
    if(!data.isEmpty() && !spill.isEmpty() && spill.contains(eventNumber+1)){
        eventNumber++;
        replot();
    }
    else if(!data.isEmpty()){
        next_spill();
    }
}

void MainWindow::next_spill(){
    if(!data.isEmpty() && data.contains(spillNumber+1)){
        spillNumber++;
        eventNumber = 0;
        replot();
    }
    else if(!data.isEmpty()){
        // requested spill does not match one in the current memory chunk
        // need to go to the next chunk of data
        spillNumber++;
        getData(spillNumber);
        replot();
    }
}

void MainWindow::previous_event(){
    if(!data.isEmpty() && !spill.isEmpty() && spill.contains(eventNumber-1)){
        eventNumber--;
        replot();
    }
    else if(!data.isEmpty()){
        previous_spill();
    }
}

void MainWindow::previous_spill(){
    if(!data.isEmpty() && data.contains(spillNumber-1)){
        spillNumber--;
        eventNumber = 0;
        replot();
    }
    else if(!data.isEmpty()){
        // requested spill does not match one in the current memory chunk
        // if requested spill > 0, we need to go to a previous chunk
        if(spillNumber-1 >= 1){
            // spill range = 100, initial spill = 0
            // first chunk: spills 0 to 99
            // second chunk: spills 100 to 199
            // current spill = 100
            // desired spill = 99
            // need to re-read the 0--99 chunk again
            // so starting spill must be at current spill - spill range = 100 - 100 = 0

            int start_spill = (spillNumber) - settings_window->GetSpillRange();
            getData(start_spill);
            spillNumber--;
            replot();
        }
    }
}

void MainWindow::choose_spill(){
    if(!data.isEmpty() && data.contains(ui->int_goToSpill->value())){
        spillNumber = ui->int_goToSpill->value();
        replot();
    }
}

void MainWindow::choose_event(){
    if(!data.isEmpty() && !spill.isEmpty() && spill.contains(ui->int_goToEvent->value())){
        eventNumber = ui->int_goToEvent->value();
        replot();
    }
}

void MainWindow::getData(int start_spill){
    if(data.isEmpty())
    {
        read_data->SetStartingSpill(start_spill);
        data = read_data->Read(filename); // start from spill 0
    }
    else{
        // we already have some data, but have requested a spill that
        // isn't in the current segment we've got in memory
        data.clear();
        read_data->SetStartingSpill(start_spill);
        data = read_data->Read(filename);
    }
}

void MainWindow::replot(){
    spillLabel = QString::number(spillNumber);
    eventLabel = QString::number(eventNumber);
    ui->label_eventNumber->setText(eventLabel);
    ui->label_spillNumber->setText(spillLabel);

    spill.clear();
    event.clear();

    spill = data.value(spillNumber);
    event = spill.value(eventNumber);

    QVector<double> x = event.at(0);
    QVector<double> y = event.at(1);
    QVector<double> z = event.at(2);
    QVector<double> t = event.at(3);
    QVector<double> px = event.at(4);
    QVector<double> py = event.at(5);
    QVector<double> pz = event.at(6);

   // QVector<double> pt, p;
   // for(int i = 0; i < px.size(); i++){
   //     pt << TMath::Sqrt(px.at(i)*px.at(i) + py.at(i)*py.at(i));
   //     p << TMath::Sqrt(px.at(i)*px.at(i) + py.at(i)*py.at(i) + pz.at(i)*pz.at(i));
   // }

    ui->plot_position_xz->graph(0)->setData(z, x);
    ui->plot_position_yz->graph(0)->setData(z, y);

    ui->plot_momentum_t->graph(0)->setData(z, px);
    ui->plot_momentum_t->graph(3)->setData(z, py);
    //ui->plot_momentum_t->graph(6)->setData(z, pt);

    ui->plot_momentum_z->graph(0)->setData(z, pz);
    //ui->plot_momentum_z->graph(3)->setData(z, p);


    // plot TOF0:
    QVector<double> tof0_x, tof0_y, tof0_z;
    tof0_x << x.at(0);
    tof0_y << y.at(0);
    tof0_z << z.at(0);

    ui->plot_position_xz->graph(1)->setData(tof0_z, tof0_x);
    ui->plot_position_yz->graph(1)->setData(tof0_z, tof0_y);

    // plot TOF1:
    QVector<double> tof1_x, tof1_y, tof1_z;
    tof1_x << x.at(1);
    tof1_y << y.at(1);
    tof1_z << z.at(1);

    ui->plot_position_xz->graph(2)->setData(tof1_z, tof1_x);
    ui->plot_position_yz->graph(2)->setData(tof1_z, tof1_y);

    // plot upstream tracker:
    QVector<double> tku_x, tku_y, tku_z, tku_px, tku_py, tku_pt, tku_pz, tku_p;
    tku_x << x.at(2) << x.at(3) << x.at(4) << x.at(5) << x.at(6);
    tku_y << y.at(2) << y.at(3) << y.at(4) << y.at(5) << y.at(6);
    tku_z << z.at(2) << z.at(3) << z.at(4) << z.at(5) << z.at(6);
    tku_px << px.at(2) << px.at(3) << px.at(4) << px.at(5) << px.at(6);
    tku_py << py.at(2) << py.at(3) << py.at(4) << py.at(5) << py.at(6);
    tku_pz << pz.at(2) << pz.at(3) << pz.at(4) << pz.at(5) << pz.at(6);
    //tku_pt << pt.at(2) << pt.at(3) << pt.at(4) << pt.at(5) << pt.at(6);
    //tku_p << p.at(2) << p.at(3) << p.at(4) << p.at(5) << p.at(6);

    ui->plot_position_xz->graph(3)->setData(tku_z, tku_x);
    ui->plot_position_yz->graph(3)->setData(tku_z, tku_y);
    ui->plot_momentum_t->graph(1)->setData(tku_z, tku_px);
    ui->plot_momentum_t->graph(4)->setData(tku_z, tku_py);
    ui->plot_momentum_z->graph(1)->setData(tku_z, tku_pz);

    // plot downstream tracker:
    QVector<double> tkd_x, tkd_y, tkd_z, tkd_px, tkd_py, tkd_pt, tkd_pz, tkd_p;
    tkd_x << x.at(7) << x.at(8) << x.at(9) << x.at(10) << x.at(11);
    tkd_y << y.at(7) << y.at(8) << y.at(9) << y.at(10) << y.at(11);
    tkd_z << z.at(7) << z.at(8) << z.at(9) << z.at(10) << z.at(11);
    tkd_px << px.at(7) << px.at(8) << px.at(9) << px.at(10) << px.at(11);
    tkd_py << py.at(7) << py.at(8) << py.at(9) << py.at(10) << py.at(11);
    tkd_pz << pz.at(7) << pz.at(8) << pz.at(9) << pz.at(10) << pz.at(11);
    //tkd_pt << pt.at(7) << pt.at(8) << pt.at(9) << pt.at(10) << pt.at(11);
    //tkd_p << p.at(7) << p.at(8) << p.at(9) << p.at(10) << p.at(11);


    ui->plot_position_xz->graph(4)->setData(tkd_z, tkd_x);
    ui->plot_position_yz->graph(4)->setData(tkd_z, tkd_y);
    ui->plot_momentum_t->graph(2)->setData(tkd_z, tkd_px);
    ui->plot_momentum_t->graph(5)->setData(tkd_z, tkd_py);
    ui->plot_momentum_z->graph(2)->setData(tkd_z, tkd_pz);

    // plot TOF2:
    QVector<double> tof2_x, tof2_y, tof2_z;
    tof2_x << x.at(12);
    tof2_y << y.at(12);
    tof2_z << z.at(12);

    ui->plot_position_xz->graph(5)->setData(tof2_z, tof2_x);
    ui->plot_position_yz->graph(5)->setData(tof2_z, tof2_y);

    ui->plot_position_xz->replot();
    ui->plot_position_yz->replot();
    ui->plot_momentum_t->replot();
    ui->plot_momentum_z->replot();
}
