#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <fft_calc.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void chooseAction();
    void FFTAction();

    void s_fftPlot(QVector<double> x, QVector<double> y);

private:
    void initButtons();

    Ui::MainWindow *ui;

    FFT_calc FFT;

    QVector<double> analyzed_data;
};
#endif // MAINWINDOW_H
