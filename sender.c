/*-----------------------------------------------------------------------------
Filename:- 	sender.c
Descritption:- This is the main side side implementation C file for the 
	 		custom network project created as part of CSCI558L lab coursework.
Date:-		Oct 9th, 2014 - 
Authors:- 	Lohith Bellad
			University of Southern California, Los Angeles, California
Platform:- 	Mac OS X Mountain Lion, Ubuntu 12.04
Place:-		Los Angeles, California
-----------------------------------------------------------------------------*/

#include "custom.h"

int main(int argc, char **argv)
{
     int sock_id = 0;
	struct ifreq ifr;
	struct sockaddr_ll sll;
	u_char buffer[2048];
	int err = 0, fd,ret,i;
	FILE *fp;
	int sum = 0;
	struct timeval tyme1,tyme2;
	unsigned long time1,time2;
	double rate;
	struct custom_hdr *header;
	unsigned short seq_no;
	int retran_limit = 8;
	
	// creating the sending socket
     sock_id = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

     memset(&ifr, 0, sizeof(ifr));
	memcpy(ifr.ifr_name, argv[1], 4);
	
     if( (err = ioctl(sock_id, SIOCGIFINDEX, &ifr)) < 0)
	{
           perror("IOCTL:");
		 close(sock_id);
           exit(1);
     }

     // binding to interface
     memset(&sll, 0, sizeof(sll));
     sll.sll_family       = AF_PACKET;
     sll.sll_ifindex      = ifr.ifr_ifindex;
     sll.sll_protocol     = htons(ETH_P_ALL);//htons(0x3333)
	
     if ( (err = bind(sock_id, (struct sockaddr *) &sll, sizeof(sll))) == -1 )
	{
        printf("Binding failed\n");
	   close(sock_id);
        exit(1);
     }
 	
	if( (fp = fopen(argv[2],"r")) == NULL)
	{
		printf("Error opening the file\n");
		exit(1);
	}
	fd = fileno(fp);
	
	sum = 0;
	/* timestamp the start time */
	if(gettimeofday(&tyme1, NULL) < 0)
	{
		printf("Error in calculating the data rate\n");
		exit(1);
	}
	seq_no = 0;
	while(1)
	{
		memset(buffer,0,sizeof(buffer));
		// Building the header for packet to nodeB
		header = (struct custom_hdr *)&buffer[0];
		header->dst_node = htons(2);
		header->src_node = htons(1);
		header->seq_num = htons(seq_no);
		
		if( (ret = read(fd,&buffer[6],1494)) < 0)
		{
			printf("Error reading from file\n");
			close(sock_id);
			exit(1);
		}
		for(i = 0; i < retran_limit; i++)
		{
			//print_payload(buffer,6);
			if( (err = send(sock_id, buffer, (ret + 6), 0)) < 0)
			{
				printf("Error in sending the packet\n");
				close(sock_id);
				exit(1);
			}
		}
		sum = sum + ret;
		// Building the header for packet to nodeC
		/*header->dst_node = htons(3);
		header->src_node = htons(1);
		header->seq_num = htons(seq_no);
		
		for(i = 0; i < 8; i++)
		{
			//print_payload(buffer,6);
			if( (err = send(sock_id, buffer, (ret + 6), 0)) < 0)
			{
				printf("Error in sending the packet\n");
				close(sock_id);
				exit(1);
			}
		}
		sum = sum + ret;*/
		
		if(ret < 1494)
			break;
		seq_no++;
	}
	
	
	if(gettimeofday(&tyme2, NULL) < 0)
	{
		printf("Error in calculating the data rate\n");
		exit(1);
	}
	time1 = tyme1.tv_usec + (tyme1.tv_sec * 1000000);
	time2 = tyme2.tv_usec + (tyme2.tv_sec * 1000000);
	time1 = time2 - time1;
	rate = (sum * 1000.0 * 8)/ time1;
	printf("\n%d bytes transferred in %5.2fsecs, data rate = %5.2fkbps\n\n",(sum),(time1/1000000.0),rate);
	close(sock_id);
	return 0;
}