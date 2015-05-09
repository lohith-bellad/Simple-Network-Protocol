/*-----------------------------------------------------------------------------
Filename:- 	aux.c
Descritption:- This is the c file containing all the auxiliary functions required
	 		for custom network project created as part of CSCI558L lab coursework.
Date:-		Oct 9th, 2014 - 
Authors:- 	Lohith Bellad
			University of Southern California, Los Angeles, California
Platform:- 	Mac OS X Mountain Lion, Ubuntu 12.04
Place:-		Los Angeles, California
-----------------------------------------------------------------------------*/

#include "custom.h"

void print_hex_ascii_line(const u_char *payload, int len, int offset){
	int i;
	int gap;
	const u_char *ch;
	/* offset */
	printf("%05d   ", offset);
	/* hex */
	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7)
			printf(" ");
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8)
		printf(" ");
	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");
	/* ascii (if printable) */
	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}
	printf("\n");
return;
}

void print_payload(const u_char *payload, int len){

	int len_rem = len;
	int line_width = 16;/* number of bytes per line */
	int line_len;
	int offset = 0;/* zero-based offset counter */
	const u_char *ch = payload;

	if (len <= 0)
		return;

	/* data fits on one line */
	if (len <= line_width) {
		print_hex_ascii_line(ch, len, offset);
		return;
	}

	/* data spans multiple lines */
	for ( ;; ) {
		/* compute current line length */
		line_len = line_width % len_rem;
		/* print line */
		print_hex_ascii_line(ch, line_len, offset);
		/* compute total remaining */
		len_rem = len_rem - line_len;
		/* shift pointer to remaining bytes to print */
		ch = ch + line_len;
		/* add offset */
		offset = offset + line_width;
		/* check if we have line width chars or less */
		if (len_rem <= line_width) {
			/* print last line and get out */
			print_hex_ascii_line(ch, len_rem, offset);
			break;
		}
	}

return;
}

// create a socket, and bind to a interface
int createSocket(const char *name){
   // create socket
   int handle = 0;
   handle = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

   // check interface
   struct ifreq ifr;
   memset(&ifr, 0, sizeof(ifr));
   if(strcmp(name, "allintf")){
      //printf("listen to all intf %s\n", name);
      strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
      if( ioctl(handle, SIOCGIFINDEX, &ifr) == -1 ){
         printf("no such device\n");
         return -1;
      }
   } else {
      printf("listen to all intf\n");
   }

   // bind interface
   struct sockaddr_ll   sll;
   memset(&sll, 0, sizeof(sll));
   sll.sll_family       = AF_PACKET;
   sll.sll_ifindex      = ifr.ifr_ifindex;
   sll.sll_protocol     = htons(ETH_P_ALL);//htons(0x3333)
   if ( bind( handle, (struct sockaddr *) &sll, sizeof(sll)) == -1 ){
      printf("fail binding\n");
      return 0;
   }
   //printf("binding intf index: %d\n", ifr.ifr_ifindex);

   // return socket
   return handle;
}

// checking interfaces for custom routing
int isRoutingPort(const char *ifName,char *control){
   return (memcmp(ifName,"lo",2) != 0) && (memcmp(ifName,control,4) != 0);
}


// function to add routing table entry
int routingTableAddEntry(struct routingTableElem *routingTable, unsigned short node_add, const char* name, int sock, int hop, int *size)
{
	routingTable[*size].node_add = node_add;
	strcpy(routingTable[*size].intfName, name);
	routingTable[*size].sockFd = sock;
	routingTable[*size].hop = hop;
	*size += 1;
	return 0;
}

// function to lookup entry in routing table
int routingTableLookUp(struct routingTableElem *routingTable, unsigned short node_add, int *size, int *sock)
{
	int i;
	for(i=0; i < *size; i++)
	{
		if( routingTable[i].node_add == node_add)
		{
			*sock = routingTable[i].sockFd;
			return 0;
		}
	}
	return -1;
}

// function to dump the routing table
int routingTableDump(struct routingTableElem *routingTable, int size){
	int i;
	printf("\n+---------------------------------------------------------+\n");
	printf("|                    ROUTING TABLE                        |\n");
	printf("+-----------------+-----------------+------------+--------+\n");
	printf("|     Node Add    |       NAME      |    SOCK    |  HOP   |\n");
	printf("+-----------------+-----------------+------------+--------+\n");
	for(i=0; i<size; i++)
	{
		printf("| %15d | %15s | %10d | %6d |\n",routingTable[i].node_add, routingTable[i].intfName, routingTable[i].sockFd, routingTable[i].hop);
	}
	printf("+---------------------------------------------------------+\n");
	return 0;
}
