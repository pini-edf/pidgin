#include "module.h"

MODULE = Gaim::Network  PACKAGE = Gaim::Network  PREFIX = gaim_network_
PROTOTYPES: ENABLE

const char *
gaim_network_get_local_system_ip(fd)
	int fd

const char *
gaim_network_get_my_ip(fd)
	int fd

unsigned short
gaim_network_get_port_from_fd(fd)
	int fd

const char *
gaim_network_get_public_ip()

void
gaim_network_init()

const unsigned char *
gaim_network_ip_atoi(ip)
	const char *ip

int
gaim_network_listen(port, socket_type)
	unsigned short port
	int socket_type

int
gaim_network_listen_range(start, end, socket_type)
	unsigned short start
	unsigned short end
	int socket_type

void
gaim_network_set_public_ip(ip)
	const char *ip
