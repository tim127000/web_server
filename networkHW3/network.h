#ifndef _network_h
#define _network_h

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<errno.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<net/ethernet.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>
#include<arpa/inet.h>
#include<time.h>
#include<pcap.h>

#define OFFMASK 0x1fff
#define MAX_IP 100
#define FILE_LEN 20
#define MAX_ADDLEN 18
typedef unsigned char u_char;
typedef unsigned int u_int;

typedef struct{

	int num;
	char src_ip[INET_ADDRSTRLEN];
	char dst_ip[INET_ADDRSTRLEN];
}Counter;

char *mac_ntoa(u_char *d);
char *ip_ttoa(u_int8_t flag);
char *ip_ftoa(u_int16_t flag);
void dump_tcp(const u_char *content);
void dump_udp(const u_char *content);
void ip_count(char *src_ip , char *dst_ip);
void record_counter();

#endif
