#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <math.h>
#include <QDebug>
#include <QFile>

#define PI 3.14159
#define RANGE_START 0
#define RANGE_END   50000

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    FFT(200000)
{
    ui->setupUi(this);
    ui->signalPlot->addGraph();
    ui->fftPlot->addGraph();
    initButtons();

    ui->fftPlot->setInteraction(QCP::iRangeDrag, true);
    ui->fftPlot->setInteraction(QCP::iRangeZoom, true);
    ui->signalPlot->setInteraction(QCP::iRangeDrag, true);
    ui->signalPlot->setInteraction(QCP::iRangeZoom, true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::initButtons()
{
    QToolBar *b_bar = ui->toolBar;

    b_bar->addAction("Выбрать файл", this, SLOT(chooseAction()));
    b_bar->addAction("Рассчитать БПФ", this, SLOT(FFTAction()));


    connect(&FFT, SIGNAL(processCompleted(QVector<double>,QVector<double>)),
            this, SLOT(s_fftPlot(QVector<double>,QVector<double>)));
}

void MainWindow::chooseAction()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Выберите файл для анализа",
                                                    QDir::currentPath() + "/analyseFiles", "(*.txt)"/*, "(*.txt)"*/);
    if( filePath.isEmpty() )
        return;

    ui->statusbar->showMessage(filePath);

    QFile datatxt(filePath);
    if (!datatxt.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    analyzed_data.clear();
    QVector<double> x;

    while(!datatxt.atEnd())
    {
        QString str = datatxt.readLine();

        QStringList lst = str.split(",");
        analyzed_data.append(lst.at(1).toDouble());
        x.append(lst.at(0).toDouble());
    }

    ui->signalPlot->graph(0)->setData(x, analyzed_data);
    ui->signalPlot->xAxis->setRange(-0.0025, 0.0025);
    ui->signalPlot->yAxis->setRange(-0.3, 0.3);
    ui->signalPlot->xAxis->setLabel("Время, с.");
    ui->signalPlot->yAxis->setLabel("Амплитуда");
    ui->signalPlot->replot();

}

void MainWindow::FFTAction()
{
    // QVector<double> data;
    // QVector<double> x;
    // double w = 2.0 * PI * 40000.0;
    // double time = 2; // in sec
    // uint fs = 200000;  // sampling frequency
    // double dt = 1.0/fs;

    // uint wave_duration = time*fs;
    // for (uint i = 0; i < wave_duration; i++) {
    //     x.append(i * dt*1000);
    //     data.append(5*sin (w * i *dt) + 10*sin(2.0 * PI * 43000 * i * dt) + 8*sin(2*PI*38000*i*dt +10));
    //     // data.append(sin (2.0 * PI * 440.0 * (float)i / (float)fs ));
    // }

    // ui->signalPlot->graph(0)->setData(x, data);
    // ui->signalPlot->xAxis->setRange(0, 0.05);
    // ui->signalPlot->yAxis->setRange(-8, 8);
    // ui->signalPlot->xAxis->setLabel("Время, мс.");
    // ui->signalPlot->yAxis->setLabel("Амплитуда");
    // ui->signalPlot->replot();

    FFT.setData(analyzed_data);
}

void MainWindow::s_fftPlot(QVector<double> x, QVector<double> y)
{
    ui->fftPlot->graph(0)->setData(x, y);
    ui->fftPlot->xAxis->setRange(RANGE_START, RANGE_END);
    ui->fftPlot->yAxis->setRange(-0.002, 0.002);
    ui->fftPlot->xAxis->setLabel("Частота, Гц.");
    ui->fftPlot->yAxis->setLabel("Амплитуда");
    ui->fftPlot->replot();
}




