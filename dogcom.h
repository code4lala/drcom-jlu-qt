#ifndef DOGCOM_H
#define DOGCOM_H

#include <QThread>
#include "interruptiblesleeper.h"
#include <QUdpSocket>
#include "constants.h"

class DogCom : public QThread
{
    Q_OBJECT
public:
    DogCom(InterruptibleSleeper *);
    void Stop();
    void FillConfig(QString a,QString p,QString m);
protected:
    void run() override;
private:
    InterruptibleSleeper *sleeper;
    QString account;
    QString password;
    QString mac_addr;

    QHostAddress *server_address;
    unsigned short port_dest;

    bool dhcp_challenge(QUdpSocket &udp_sender,unsigned char seed[]);
    bool log;
    void print_packet(const char msg[10], const unsigned char *packet, int length);
    int dhcp_login(QUdpSocket &udp_sender,unsigned char seed[],unsigned char auth_information[]);
    int keepalive_1(QUdpSocket &udp_sender, unsigned char auth_information[]);
    int keepalive_2(QUdpSocket &udp_sender, int *keepalive_counter, int *first);
    void gen_crc(unsigned char seed[], int encrypt_type, unsigned char crc[]);
    void keepalive_2_packetbuilder(unsigned char keepalive_2_packet[], int keepalive_counter, int filepacket, int type);


signals:
    void ReportOffline(int reason);
    void ReportOnline();
    void ReportIpAddress(unsigned char x1,unsigned char x2,unsigned char x3,unsigned char x4);
};

#endif // DOGCOM_H
