#ifndef MIM_COMMON_H
#define MIM_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

uint16_t read_port(char const *string);
struct sockaddr_in get_address(char const *host, uint16_t port);
int safe_socket(int domain, int type, int protocol);
void safe_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
size_t safe_recvfrom(int sockfd, void *buf, size_t len, int flags,
                      struct sockaddr *src_addr, socklen_t *addrlen);
void safe_sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
void install_signal_handler(int signal, void (*handler)(int), int flags);

#endif
