#ifndef NETWORK_H
#define NETWORK_H

#include <QThread>
#include <QObject>
#include <QMultiMap>
#include <QtNetwork>

#include <stdio.h>
#include <winsock2.h> //Исключение переопределения
#include <windows.h>

#define HAVE_REMOTE


#include "pcap.h"
#include "datasource.h"

class Network: public DataSource
{
    Q_OBJECT
public:
    explicit Network();
    virtual ~Network();
    int open(const QString str)override;
public slots:
    void run()override;
protected:
    QString _name;
    bool _isOpen;
    pcap_t *_fp;
private:
    virtual int init(const QString &str);
    int initDev(char *ip);

    QUdpSocket *_sock;
};

class Dumpwork : public Network
{
    Q_OBJECT
public:
    Dumpwork(uint32_t maxPacketCount);
public slots:
    void run()override;
private:
    uint32_t _maxPacketCount;
    virtual int init(const QString &str);
};
#endif // NETWORK_H
