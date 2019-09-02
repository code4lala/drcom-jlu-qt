#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#endif
#include <string>
#include <time.h>
#include <QDebug>
#include "DogcomSocket.h"

using std::string;

char DogcomSocket::AUTH_SERVER[20] = "10.100.61.3";

DogcomSocket::DogcomSocket() {
}

void DogcomSocket::init(){
    int r;
#ifdef WIN32
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsadata;
    if ((r = WSAStartup(sockVersion, &wsadata)) != 0) {
        throw DogcomSocketException(DogcomError::WSA_START_UP, r);
    }
#endif

    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
#ifdef WIN32
    bind_addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
#else
    bind_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
#endif
    bind_addr.sin_port = htons(BIND_PORT);

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
#ifdef WIN32
    dest_addr.sin_addr.S_un.S_addr = inet_addr(AUTH_SERVER);
#else
    dest_addr.sin_addr.s_addr = inet_addr(AUTH_SERVER);
#endif
    dest_addr.sin_port = htons(DEST_PORT);

    srand(time(nullptr));

    // create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
#ifdef WIN32
        throw DogcomSocketException(DogcomError::SOCKET, WSAGetLastError());
#else
        throw DogcomSocketException(DogcomError::SOCKET,sockfd);
#endif
    }

    // bind socket
    if (bind(sockfd, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) < 0) {
#ifdef WIN32
        throw DogcomSocketException(DogcomError::BIND, WSAGetLastError());
#else
        throw DogcomSocketException(DogcomError::BIND,sockfd);
#endif
    }

    // set timeout
#ifdef WIN32
    int timeout = 3000;
#else
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
#endif
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout,
                   sizeof(timeout)) < 0) {
#ifdef WIN32
        throw DogcomSocketException(DogcomError::SET_SOCK_OPT_TIMEOUT,
                                    WSAGetLastError());
#else
        throw DogcomSocketException(DogcomError::SET_SOCK_OPT_TIMEOUT,
                sockfd);
#endif
    }

    //set port reuse

    int optval = 1;
#ifdef WIN32
    if ((r = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof(optval))) < 0) {
        throw DogcomSocketException(DogcomError::SET_SOCK_OPT_REUSE, r);
        }
#else
    if ((r = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof(optval))) < 0) {
        throw DogcomSocketException(DogcomError::SET_SOCK_OPT_REUSE,r);
    }

#ifdef SO_REUSEPORT
    if ((r = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (char *) &optval, sizeof(optval))) < 0) {
        throw DogcomSocketException(DogcomError::SET_SOCK_OPT_REUSE,r);
    }
#endif
#endif
}

int DogcomSocket::write(const char *buf, int len) {
    return sendto(sockfd, buf, len, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
}

int DogcomSocket::read(char *buf) {
#ifdef WIN32
    int addrlen = sizeof(dest_addr);
#else
    socklen_t addrlen=sizeof(dest_addr);
#endif
    return recvfrom(sockfd, buf, 1024, 0, (struct sockaddr *) &dest_addr, &addrlen);
}

DogcomSocket::~DogcomSocket() {
    qDebug()<<"DogcomSocket destructor";
#ifdef WIN32
    if (sockfd != -1)
        closesocket(sockfd);
    WSACleanup();
#else
    if (sockfd != -1)
        close(sockfd);
#endif
    qDebug()<<"socket closed";
}

const char *DogcomSocketException::what() const noexcept {
    static char buf[1024];
    switch (errCode) {
        case DogcomError::WSA_START_UP:
            snprintf(buf, sizeof(buf),
                     "WSAStartup failed. Error code: %d",
                     realErrCode);
            break;
        case DogcomError::SOCKET:
            snprintf(buf, sizeof(buf),
                    "socket failed. Error code: %d",
                    realErrCode);
            break;
        case DogcomError::BIND:
            snprintf(buf, sizeof(buf),
                    "bind failed. Error code: %d",
                    realErrCode);
            break;
        case DogcomError::SET_SOCK_OPT_TIMEOUT:
            snprintf(buf,sizeof(buf),
                    "timeout failed. Error code: %d",
                    realErrCode);
            break;
        case DogcomError::SET_SOCK_OPT_REUSE:
            snprintf(buf, sizeof(buf),
                    "port reuse failed. Error code: %d",
                    realErrCode);
            break;
    }
    return buf;
}

DogcomSocketException::DogcomSocketException(int errCode, int realErrCode)
        : errCode(errCode), realErrCode(realErrCode) {
}
