#include "network.h"

#include <QThread>
#include <QMutex>
#include <QMutexLocker>


/* 4 bytes IP address */
typedef struct ip_address{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header{
    u_char	ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
    u_char	tos;			// Type of service
    u_short tlen;			// Total length
    u_short identification; // Identification
    u_short flags_fo;		// Flags (3 bits) + Fragment offset (13 bits)
    u_char	ttl;			// Time to live
    u_char	proto;			// Protocol
    u_short crc;			// Header checksum
    ip_address	saddr;		// Source address
    ip_address	daddr;		// Destination address
    u_int	op_pad;			// Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header{
    u_short sport;			// Source port
    u_short dport;			// Destination port
    u_short len;			// Datagram length
    u_short crc;			// Checksum
}udp_header;

 //Function prototypes
char *iptos(u_long in);
/* prototype of the packet handler */
int packet_handler( const struct pcap_pkthdr *header, const u_char *pkt_data, Ndata &ndt);

bool check_addr(pcap_if_t *d, char *ip)
{
    pcap_addr_t *a;
    for(a=d->addresses;a;a=a->next)
    {
        switch(a->addr->sa_family)
        {
        case AF_INET:
            if (a->addr)
            {
                if( strcmp( ip, iptos( ( ( struct sockaddr_in *)a->addr)->sin_addr.s_addr)) == 0  ) return true;
            }
            break;
        default:
            break;
        }
    }
    return false;
}

/* From tcptraceroute, convert a numeric IP address to a string */
#define IPTOSBUFFERS	12
char *iptos(u_long in)
{
    static char output[IPTOSBUFFERS][3*4+3+1];
    static short which;
    u_char *p;

    p = (u_char *)&in;
    which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
    _snprintf_s(output[which], sizeof(output[which]), sizeof(output[which]), "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return output[which];
}

int Network::initDev(char* ip)
{
    pcap_if_t *alldevs, *d;
    uint32_t inum, i=0;
    char errbuf[PCAP_ERRBUF_SIZE];

    /* The user didn't provide a packet source: Retrieve the local device list */
    if (pcap_findalldevs_ex((char*)PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
    {
        fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
        return -1;
    }

    inum = 0;
    for(d=alldevs; d; d=d->next)
    {
        ++i;
        if( check_addr( d, ip ) )
        {
            inum = i;
            break;
        }
    }

    if( inum == 0 )
    {
        fprintf(stderr,"\nError find adapter\n");
        return -1;
    }
    /* Jump to the selected adapter */
    for (d=alldevs, i=0; i< inum-1 ;d=d->next, i++);

    /* Open the device */
    if ( (_fp = pcap_open(d->name,
                         200/*50*//*250*//*100*/ /*snaplen*/,
                         PCAP_OPENFLAG_PROMISCUOUS /*flags*/,
                         20/*20*/ /*read timeout*/,
                         NULL /* remote authentication */,
                         errbuf)
          ) == NULL)
    {
        fprintf(stderr,"\nError opening adapter\n");
        return -1;
    }
    return 0;
}

int packet_handler( const struct pcap_pkthdr *header, const u_char *pkt_data, Ndata &ndt)
{
    ip_header *ih;
    udp_header *uh;
    u_int ip_len;

    /* retireve the position of the ip header */
    ih = (ip_header *) (pkt_data +
                        14); //length of ethernet header

    /* retireve the position of the udp header */
    ip_len = (ih->ver_ihl & 0xf) * 4;
    uh = (udp_header *) ((u_char*)ih + ip_len);

    /* convert from network byte order to host byte order */
    ndt.dport = ntohs( uh->dport );
    ndt.sport = ntohs( uh->sport );

    ndt.address.setAddress(
                QString("%1.%2.%3.%4")
                .arg(ih->saddr.byte1)
                .arg(ih->saddr.byte2)
                .arg(ih->saddr.byte3)
                .arg(ih->saddr.byte4)
                );

    size_t /*int*/ size = header->len - 14 - ip_len - sizeof(udp_header);

    ndt.datagram = QByteArray( (char*)&pkt_data[ 14 + ip_len + sizeof(udp_header)], size );

    return 0;
}

/**
 * @brief Network Конструктор класса
 */
Network::Network()
{
}

/**
 * @brief Network::open задать сетевой интерфейс
 * @param name название сетевого интерфейса
 * @return код ошибки
 */
int Network::open(const QString str)
{
    int res = -1;

    if(!str.isEmpty())
    {
        _name = str;
        res = 0;
    }

    return res;
}
/**
 * @brief init Подключиться к выбранному сетевому устройству
 * @param net ip выбранного сетевого подключения
 * @return результат выполнеия операции
 */
int Network::init(const QString &str)
{
    int ret = -1;
    if( str.length() ) ret = initDev(str.toLocal8Bit().data());
    return ret;
}

/**
 * @brief Network::run рабочий поток
 */
void Network::run()
{
    running = true;
    if( init(_name) == 0 )
    {
        int res;
        struct pcap_pkthdr *header;
        const uint8_t *pkt_data;

        Ndata data;
        while ( running )
        {
            while( !stopped && (res = pcap_next_ex( _fp, &header, &pkt_data)) >= 0 )
            {
                if(res == 0)continue;
                packet_handler(header, pkt_data, data);
                _time = (uint64_t)(header->ts.tv_sec) * 1000 +  ( header->ts.tv_usec) * 0.001;

                add(data);
            }
        }
    }
    emit finished();
}

/**
 * @brief ~Network Деструктор класса
 */
Network::~Network()
{
}

Dumpwork::Dumpwork(uint32_t maxPacketCount):
    _maxPacketCount(maxPacketCount){}

int Dumpwork::init(const QString &str)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    _fp = pcap_open_offline(str.toLocal8Bit().data(), errbuf);
    return ( _fp == nullptr ? -1 : 0 );
}

void Dumpwork::run()
{

    running = true;

    if( init(_name) == 0 )
    {
        int res;
        struct pcap_pkthdr *header;
        const uint8_t *pkt_data;

        Ndata data;
        while ( running )
        {
            while( !stopped && (res = pcap_next_ex( _fp, &header, &pkt_data)) >= 0 )
            {
                if(res == 0) {running  = false;break;};

                packet_handler(header, pkt_data, data);
                _time = (header->ts.tv_sec) * 1000 +  ( header->ts.tv_usec) / 1000;
                add(data);
                if( count() > _maxPacketCount)stopped  = true;
            }
        }
    }
    emit finished();
}
