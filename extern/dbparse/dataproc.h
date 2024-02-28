#ifndef DATAPROC_H
#define DATAPROC_H

#include <mutex>

#include <QObject>

#include "datasource.h"


class DataProc : public QObject
{
    Q_OBJECT

public:
    explicit DataProc(DataSource * net,  const QMultiMap<int, param_stg_t> &param_stg, QObject *parent = nullptr );
    void setParams(const QMultiMap<int, param_stg_t> &param_stg);
    inline void setTimeout(const uint32_t timeout){m_timeout = timeout;}
    inline void setNetDev(DataSource* net){m_net = net;}

signals:
    void finished();
    void update_graph( int ch, const QVector<double> data, unsigned int time);
    void update();
public slots:
    inline void stop(){m_running = false;}
    void run();
private:
    void packetproc(const QList<Ndata> &ldata, unsigned int timeout);

    uint32_t m_timeout;
    volatile bool m_running;
    DataSource *m_net;
    QMultiMap<int, param_stg_t> m_param_stg;
    std::mutex m_mtx;
};

#endif // DATAPROC_H
