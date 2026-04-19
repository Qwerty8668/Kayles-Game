#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>
#include <cinttypes>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "err.h"
#include "common.h"

#include <chrono>

std::chrono::seconds read_timeout(char const *t) {
    char *endptr;
    errno = 0;
    unsigned long timeout = strtoul(t, &endptr, 10);
    if (errno != 0 || *endptr != 0 || timeout > 99 || timeout == 0) {
        fatal("%s is not valid timeout", t);
    }
    std::chrono::seconds ret(timeout);
    return ret;
}

uint16_t read_port(char const *string) {
    char *endptr;
    errno = 0;
    unsigned long port = strtoul(string, &endptr, 10);
    if (errno != 0 || *endptr != 0 || port > UINT16_MAX) {
        fatal("%s is not a valid port number", string);
    }
    return static_cast<uint16_t>(port);
}

struct sockaddr_in get_address(char const *host, uint16_t port) {
    struct addrinfo hints{};
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;
    int errcode = getaddrinfo(host, nullptr, &hints, &address_result);
    if (errcode != 0) {
        fatal("getaddrinfo: %s", gai_strerror(errcode));
    }

    struct sockaddr_in send_address{};
    send_address.sin_family = AF_INET; // IPv4
    send_address.sin_addr.s_addr = // IP address
            reinterpret_cast<struct sockaddr_in *>(address_result->ai_addr)->sin_addr.s_addr;
    send_address.sin_port = htons(port); // port from the command line

    freeaddrinfo(address_result);

    return send_address;
}

int safe_socket(int domain, int type, int protocol) {
    int fd = socket(domain, type, protocol);
    if (fd < 0) {
        syserr("socket");
    }
    return fd;
}

void safe_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (bind(sockfd, addr, addrlen) < 0) {
        syserr("bind");
    }
}

size_t safe_recvfrom(int sockfd, void *buf, size_t len, int flags,
                      struct sockaddr *src_addr, socklen_t *addrlen) {
    ssize_t received;
    do {
        received = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    } while (received < 0 && (errno == EINTR || errno == ECONNREFUSED));

    if (received < 0) {
        syserr("recvfrom returned an unexpected error");
    }
    return received;
}

void safe_sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen) {
    if (sendto(sockfd, buf, len, flags, dest_addr, addrlen) < 0) {
        printf("Sending message failed. Errno: %d", errno);
    }
}

void safe_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    int res = setsockopt(sockfd, level, optname, optval, optlen);
    if (res < 0) {
        syserr("setsockopt");
    }
}

void safe_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = connect(sockfd, addr, addrlen);
    if (res < 0) {
        syserr("connect");
    }
}

ssize_t safe_timeout_recv(int sockfd, void *buf, size_t len, int flags) {
    ssize_t res = recv(sockfd, buf, len, flags);

    if (res < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -1;
        } else {
            syserr("recv");
        }
    }

    return res;
}
