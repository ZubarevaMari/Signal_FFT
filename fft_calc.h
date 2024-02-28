#ifndef FFT_CALC_H
#define FFT_CALC_H

#include <QObject>

#include<complex.h>
#include"extern/fftw/fftw3.h"
#include <QtMath>

#define REAL    0
#define IMG     1

class FFT_calc : public QObject
{
    Q_OBJECT
public:
    explicit FFT_calc(uint samples, QObject *parent = nullptr);

    ~FFT_calc();

    void setData(QVector<double> data, double time=1);

signals:
    void finish();
    void processCompleted(QVector<double> t_vectorX, QVector<double> t_vectorY);


private:
    void run(QVector<double> fftIn, uint size);
    void fftInit(uint len);

    double time_;

    uint sample_num;

    fftw_plan fftPlan;
    fftw_complex *fftOut;

    QVector<double> resVal;
    QVector<double> xAxis;
};

#endif // FFT_CALC_H
