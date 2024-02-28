#include <QMutexLocker>

#include "datasource.h"

constexpr auto INVAL_VALUE = 0xFFFFFFFFul;

const QList<Ndata> DataSource::getData()
{
    QMutexLocker locker(&_mutex);
    _elapsed = _timeCounter.elapsed();
    _timeCounter.stop();
    return _receiveData;

//    QList<Ndata> data = _receiveData;
//    _receiveData.clear();
//    return data;
}

/**
 * @brief clearData Очистить контейнер с принятыми данными
 */
void DataSource::clear()
{
    QMutexLocker locker(&_mutex);
    _receiveData.clear();
}


void DataSource::add(const Ndata &data)
{
    QMutexLocker locker(&_mutex);
    _receiveData << data;
    _timeCounter.add(_time);
}

size_t DataSource::count()
{
    QMutexLocker locker(&_mutex);
    return _receiveData.size();
}

void DataSource::stop()
{
    stopped = true;
}

void DataSource::start()
{
    clear();
    stopped = false;
}

void DataSource::close()
{
    if( !stopped )stop();
    running = false;
    _timeCounter.clear();
}

std::map<uint8_t, sourceNET_stg_t> getSources(QMap<int, sourceNET_stg_t > sources)
{
    std::map<uint8_t, sourceNET_stg_t> result;
    for(auto it:sources)
    {
        uint8_t ip = QHostAddress(it.ip).toIPv4Address() & 0xFF;
        result.emplace(std::make_pair(ip, it));
    }
    return result;
}


TimeCounter::TimeCounter():
    _start(INVAL_VALUE),
    _end(INVAL_VALUE)
{}

void TimeCounter::add(uint64_t time)
{
    if(_start == INVAL_VALUE)
        _start = time;
    else _end = time;
}

void TimeCounter::clear()
{
    _start = INVAL_VALUE;
    _end = INVAL_VALUE;
}

void TimeCounter::stop()
{
    _start = _end;
}

uint64_t TimeCounter::elapsed()
{
    if(_end == INVAL_VALUE) return 0;
    return _end - _start;
}
