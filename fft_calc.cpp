#include "fft_calc.h"

#include <qdebug.h>

#define DATA_SAMPLING_FREQ 200000

FFT_calc::FFT_calc(uint samples, QObject *parent)
    : QObject{parent},
    sample_num(samples)
{
    fftInit(samples);
}

FFT_calc::~FFT_calc()
{
    fftw_free(fftOut);
    fftw_destroy_plan(fftPlan);
}

void FFT_calc::fftInit(uint len)
{
    fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);

    xAxis.resize(len/2);
    for (quint32 i=0;i<len/2;i++)
    {
        xAxis[i] = i * ((double)DATA_SAMPLING_FREQ/len);
    }
}

void FFT_calc::setData(QVector<double> data, double time)
{
    time_ = time;

    if(data.empty()) return;

    static QVector<double> fftIn;

    for(int i = 0; i < data.size(); i++)
    {
        fftIn.append(data[i]);

        if(((uint)fftIn.size() >= sample_num) || (fftIn.size() >= data.size())){
            run(fftIn, fftIn.size());
            if((uint)fftIn.size() < sample_num) fftInit(fftIn.size());
            emit processCompleted(xAxis, resVal);
            return;
        }
    }
}

void FFT_calc::run(QVector<double> fftIn, uint size)
{
    QVector<double> psd_amp(size);
    for (quint32 i = 0;i < size;i ++)
    {
        psd_amp[i] = 0.5*(1-qCos(2*M_PI*i/(size-1)))*qSqrt(1.633);
        fftIn[i]=fftIn[i]*psd_amp[i];
    }
    fftPlan = fftw_plan_dft_r2c_1d(size, &fftIn[0], fftOut, FFTW_ESTIMATE);
    fftw_execute(fftPlan);

    QVector<double> ampl(size/2);

    for (uint i = 0;i < size/2;i ++)
    {
        ampl[i] = qSqrt(((fftOut[i][0]*fftOut[i][0]+fftOut[i][1]*fftOut[i][1]))
                               /(size*DATA_SAMPLING_FREQ/4));
    }
    fftw_destroy_plan(fftPlan);

    resVal.resize(ampl.size());

    for (int i=0;i<resVal.size();i++)
    {
        resVal[i] = ampl[i];
    }

}
