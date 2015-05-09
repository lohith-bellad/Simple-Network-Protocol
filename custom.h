/*-----------------------------------------------------------------------------
Filename:- 	custom.h
Descritption:- This is tsys/he header file for custom network project created as 
			part of CSCI558L lab coursework.
Date:-		Oct 9th, 2014 - 
Authors:- 	Lohith Bellad
			University of Southern California, Los Angeles, California
Platform:- 	Mac OS X Mountain Lion, Ubuntu 12.04
Place:-		Los Angeles, California
-----------------------------------------------------------------------------*/

#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <sys/types.h>//get all interface
#include <ifaddrs.h>//NI_MAXHOST
#include <netdb.h>
#include <netinet/if_ether.h>//struct ether_header
#include <netinet/ip.h>//struct ip
#include <ctype.h>
#include <arpa/inet.h>
#include <linux/icmp.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define NIPQUAD(addr) \
  ((unsigned char *)&addr)[0], \
  ((unsigned char *)&addr)[1], \
  ((unsigned char *)&addr)[2], \
  ((unsigned char *)&addr)[3] 

#define MAC_ADDR(addr) \
  (unsigned char) addr[0], \
  (unsigned char) addr[1], \
  (unsigned char) addr[2], \
  (unsigned char) addr[3], \
  (unsigned char) addr[4], \
  (unsigned char) addr[5] 
	  
// basic element for interface <-> socket table
struct ll_addr
{
  	struct ifaddrs *if_ad;
	int sock_id;
};

// basic element for routing table
struct routingTableElem{
	// node address
	unsigned short node_add; 
	// eth intf name <- maybe stored for printing
	char intfName[IFNAMSIZ];
	// eth intf sock fd <- get from intfSockTable
	int sockFd;
	// hop count
	int hop;
};

// custom packet header
struct custom_hdr{
	short dst_node;
	short src_node;
	unsigned short seq_num;
};



// some useful functions, pretty much useful!!!
void print_hex_ascii_line(const u_char *payload, int len, int offset);
void print_payload(const u_char *payload, int len);

int createSocket(const char *name);
int isRoutingPort(const char *ifName,char *control);
int routingTableAddEntry(struct routingTableElem *routingTable, unsigned short node_add, const char* name, int sock, int hop, int *size);
int routingTableLookUp(struct routingTableElem *routingTable, unsigned short node_add, int *size, int *sock);
int routingTableDump(struct routingTableElem *routingTable, int size);