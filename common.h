#ifndef MIM_COMMON_H
#define MIM_COMMON_H
#include <chrono>
#include <cstdint>
#include <sys/types.h>

std::chrono::seconds read_timeout(char const *t);

uint16_t read_port(char const *string);

struct sockaddr_in get_address(char const *host, uint16_t port);

int safe_socket(int domain, int type, int protocol);

void safe_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

size_t safe_recvfrom(int sockfd, void *buf, size_t len, int flags,
                      struct sockaddr *src_addr, socklen_t *addrlen);

void safe_sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

void safe_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

void safe_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

ssize_t safe_timeout_recv(int sockfd, void *buf, size_t len, int flags);

void install_signal_handler(int signal, void (*handler)(int), int flags);

#endif
