#ifndef DATASOURCE_H
#define DATASOURCE_H
#include <mutex>
#include <QObject>

#include <QMutex>

#include <QHostAddress>
//#include <QFile>
//#include <fstream>

//#include "higreader.h"


// Database source table data STAND
struct source_stg_t
{
    QHostAddress ip;
    int source;
    QString name;
    int canal_number;
    int size;
    bool compare_value;
    bool not_generate;
    bool is_clipped_signal;
};

struct ece_stg_t
{
    ulong data[256] = {0};
    double frequency;
    int pack_count;
    int data_count;
    int delay;
};

// Database setting table data
struct setting_stg_t
{
    int setup_pause;
    QString commut_cmd;
    uint16_t commut_pause;
    QString gener_cmd;
    uint16_t gener_pause;
    ece_stg_t ece;
    source_stg_t source;
    int source_pause;
    double source_valid_diff;
    long expected_val_channel;
    uint16_t multim_measure;
    QString type_processing;
    double max_limit;
    double min_limit;
    int multim_pause;
    QString comment;
    QString user_mess;
    bool user_need;
    bool is_used = true;
};

struct sourceNET_stg_t
{
    QString name;
    QHostAddress ip;
    uint32_t src_port;
    uint32_t dst_port;
    size_t size;
};


struct param_stg_t
{
    QString name;
    sourceNET_stg_t source;
    int address;
    int index;
    int step;
    int size;
    int freq;
    int count;
    bool order;
    uint8_t start_bit;
    uint8_t end_bit;
    bool sign;
    double cmp;
    double calib;
};


struct Ndata
{
    QHostAddress address;
    quint16 sport;
    quint16 dport;
    QByteArray datagram;
};

class TimeCounter
{
public:
    TimeCounter();
    void add(uint64_t time);
    void clear();
    void stop();
    uint64_t elapsed();
private:
    uint64_t _start;
    uint64_t _end;
};

class DataSource : public QObject
{
    Q_OBJECT
public:
    virtual ~DataSource() = default;

    virtual int open(const QString str) = 0;
    virtual void close();
    virtual void stop();
    virtual void start();
    const QList<Ndata> getData();
    uint64_t getTimeout(){return _elapsed;}
public slots:
    virtual void run() = 0;
signals:
    void finished();
protected:
    void clear();
    void add( const Ndata &data);
    size_t count();

    volatile bool stopped;
    volatile bool running;

    TimeCounter _timeCounter;
    uint64_t _elapsed;
    uint64_t _time;
private:
    QList<Ndata> _receiveData;
    mutable QMutex _mutex;

};

#endif // DATASOURCE_H
