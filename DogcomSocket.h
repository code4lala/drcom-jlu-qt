#ifndef MYUDPSOCKET_DOGCOMSOCKET_H
#define MYUDPSOCKET_DOGCOMSOCKET_H

#include <exception>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#endif

namespace DogcomError {
    enum DogcomSocketError {
        WSA_START_UP = 0x10,
        SOCKET,
        BIND,
        SET_SOCK_OPT_TIMEOUT,
        SET_SOCK_OPT_REUSE
    };
}

class DogcomSocketException : public std::exception {
public:
    int errCode = -1;
    int realErrCode = -1;
    const char *what() const noexcept override;
    DogcomSocketException(int errCode, int realErrCode);
};

class DogcomSocket {
private:
    int sockfd = -1;
    struct sockaddr_in bind_addr;
    struct sockaddr_in dest_addr;
    const static int BIND_PORT = 61440;
    const static int DEST_PORT = 61440;
    static char AUTH_SERVER[20];

public:
    DogcomSocket();
    void init();
    int write(const char *buf, int len);
    int read(char *buf);

    virtual ~DogcomSocket();

};

#endif //MYUDPSOCKET_DOGCOMSOCKET_H
