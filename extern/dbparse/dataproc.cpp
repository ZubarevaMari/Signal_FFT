#include "dataproc.h"

#include <QThread>
#include <QtEndian>
#include <QTime>
#include <QElapsedTimer>
#include <QMessageBox>

#define EXTRACT_WORD( word, start_bit, end_bit )\
    ( ( (word) << ( 64 - (end_bit) ) ) >> ( 64 - (end_bit) + ( (start_bit)  - 1 ) ) )

template <class Type> Type get_param_data(const Type *data, const int order,
                                          const uint8_t start_bit, const  uint8_t end_bit)
{
    Type word = static_cast <Type>( ( order ) ? ( qFromBigEndian( (Type) *data ) ) : ( *data ) );
    return static_cast <Type> (EXTRACT_WORD( (uint64_t)word, start_bit, end_bit ));
}

/**
 * шаблонная функция доступа к элементу по его наименованию
 */
template < class  Type> Type getParamByTitle( const Type begin,  const Type end, const QString title )
{
    Type it = begin;
    for(; it!=end; ++it )
        if( it->name == title )
            break;
    return it;
}

template <class Type, class ResType> QVector<ResType> getParams( const Type &param, const char *data, const size_t &size)
{
    auto frequency = param.count;
    auto index = param.address;
    auto displace = param.step;
    auto cmp = param.cmp;
    auto order = param.order;
    auto start_bit = param.start_bit;
    auto end_bit = param.end_bit;
    auto psize = param.size;
    auto sign = param.sign;
    auto calib = param.calib;
    uint64_t word = 0;

    QVector<ResType> res(frequency);
    ResType val = 0;
    for ( auto fc = 0; fc < frequency; fc++ )
    {
        if((  index - 1  + (frequency - 1 ) * (uint32_t)displace ) + psize  <= size )
        {
            switch ( psize )
            {
            case 1:
            {
                word = (char)EXTRACT_WORD( (uint64_t)data[ index - 1 + fc * displace ], start_bit, end_bit );
                val  = sign ? (char)word : (unsigned char) word;
                break;
            }
            case 2:
            {

                word = (short)get_param_data<short>( (short*)&data[ index - 1 + fc * displace ], order, start_bit, end_bit );
                val  = sign ? (short)word : (unsigned short) word;
                break;
            }
            case 4:
            {
                word = get_param_data<int>( (int*)&data[ index - 1 + fc * displace ], order, start_bit, end_bit );
                val  = sign ?(int)word : (unsigned int) word;
                break;
            }
            case 8:
            {
                word = get_param_data<int64_t>( (int64_t*)&data[ index - 1 + fc * displace ], order, start_bit, end_bit );
                val  = sign ? (int64_t)word : (uint64_t) word;
                break;
            }
            default:
                val = 0;
                break;
            }
            res[fc] = (ResType)val * cmp + calib;
        }
    }
    return res;
}

DataProc::DataProc(DataSource *net, const QMultiMap<int, param_stg_t> &param_stg, QObject *parent ):
    QObject(parent),
    m_timeout(1000),
    m_running(false),
    m_net(net),
    m_param_stg(param_stg)
{
    qRegisterMetaType<QVector<double>>("QVector<double>");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<source_stg_t>("source_stg_t");
    qRegisterMetaType<QMessageBox::Icon>("QMessageBox::Icon");
}


void DataProc::setParams(const QMultiMap<int, param_stg_t> &param_stg)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_param_stg = param_stg;
}

void DataProc::run()
{
    if( m_net == nullptr ) return;

    QElapsedTimer tm;

    tm.start();

    m_running = true;

    int delay = 0;
    int timeout = m_timeout;

    while ( m_running )
    {
        if(  tm.elapsed() >= timeout )
        {
            m_net->stop();
            QList<Ndata> list = m_net->getData();
            auto time = m_net->getTimeout();
            if(time == 0) time = timeout;
            m_net->start();

            packetproc(list,time);

            delay = ( tm.elapsed() - m_timeout );
            tm.restart();

            //qDebug()<<"list"<<list.size();

            if( timeout > delay )timeout -= delay;
            else timeout = 0;
        }
        QThread::usleep(timeout);
    }
    emit finished();
}


void DataProc::packetproc(const QList<Ndata> &ldata, unsigned int timeout)
{
    QMap<uint16_t, QVector<double>> chData;
    for( auto dgm : ldata )
    {
        QHostAddress ip = dgm.address;
        //auto sport = dgm.sport;
        auto dport = dgm.dport;

        std::lock_guard<std::mutex> lock(m_mtx);
        for( auto it = m_param_stg.begin(); it != m_param_stg.end(); ++it )
        {
            int key = it.key();
            auto param = it.value();
            auto source = param.source;

            if( source.ip == ip && /*source.src_port == sport &&*/
                                       source.dst_port == dport)
                chData[key].append( getParams<param_stg_t, double>( param, dgm.datagram.data(),
                                                                    dgm.datagram.size()) );
        }
    }
    for( auto key : chData.keys() ){
        emit update_graph( key, chData[key],timeout );
    }
    emit update();
}

