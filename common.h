#ifndef MIM_COMMON_H
#define MIM_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

uint16_t read_port(char const *string);
struct sockaddr_in get_server_address(char const *host, uint16_t port);
int safe_socket(int domain, int type, int protocol);
void safe_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t safe_recvfrom(int sockfd, void *buf, size_t len, int flags,
                      struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t	readn(int fd, void *vptr, size_t n);
ssize_t	writen(int fd, const void *vptr, size_t n);
void install_signal_handler(int signal, void (*handler)(int), int flags);

#endif
