/*-----------------------------------------------------------------------------
Filename:- 	receiver.c
Descritption:- This is the main receiver side implementation C file for the 
	 		custom network project created as part of CSCI558L lab coursework.
Date:-		Oct 9th, 2014 - 
Authors:- 	Lohith Bellad
			University of Southern California, Los Angeles, California
Platform:- 	Mac OS X Mountain Lion, Ubuntu 12.04
Place:-		Los Angeles, California
-----------------------------------------------------------------------------*/

#include "custom.h"

int main(int argc, char *argv[] )
{
     int sock_id;
	struct sockaddr_ll sll;
	struct ifreq ifr;
	int err = 0, ret = 0;
	u_char recv_buffer[2048];
	int sum=0,fd1,fd2,flag;
	FILE *fp1,*fp2;
	struct timeval tyme1,tyme2;
	unsigned long time1,time2;
	double rate;
	int break_cnt = 0;
	unsigned short seq_no = 0;
	struct custom_hdr *recv_hdr;
	
	// create raw socket that listens on particular interface...
     sock_id = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
     memset(&ifr, 0, sizeof(ifr));
     memset(&sll, 0, sizeof(sll));
	
	memcpy(ifr.ifr_name,argv[1],5);
	// binding socket to particular interface
	if( (err = setsockopt(sock_id, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr))) < 0)
	{
	    perror("ERROR:");
	    close(sock_id);
	    exit(1);
	}
	if (ioctl(sock_id, SIOCGIFINDEX, &ifr) < 0)
	{
	        perror("ioctl:");
	        exit(1);
	}
     sll.sll_family    = AF_PACKET;
     sll.sll_protocol  = htons(ETH_P_ALL);//htons(0x3333)
	sll.sll_ifindex   = ifr.ifr_ifindex;
     if( (bind(sock_id, (struct sockaddr *) &sll, sizeof(sll))) < 0)
     {
        perror("Failed Binding:\n");
	   close(sock_id);
	   exit(1);
     }
	
	/*--------------------------change here for the demo------------------------------*/
	if( (fp1 = fopen("test_recv_nodeB.txt","w")) == NULL)
	{
		printf("Error creating the file\n");
		exit(1);
	}
	fclose(fp1);
	
	if( (fp2 = fopen("test_recv_nodeC.txt","w")) == NULL)
	{
		printf("Error creating the file\n");
		exit(1);
	}
	fclose(fp2);
	
	if( (fp1 = fopen("test_recv_nodeB.txt","a")) == NULL)
	{
		printf("Error opening the file\n");
		exit(1);
	}
	fd1 = fileno(fp1);

	if( (fp2 = fopen("test_recv_nodeC.txt","a")) == NULL)
	{
		printf("Error opening the file\n");
		exit(1);
	}
	fd2 = fileno(fp1);
		
	printf("\nReceiver is listening\n");
	seq_no = 0;
	sum = 0;
	flag = 0;
	break_cnt = 0;
	while(1)
	{
		memset(recv_buffer,0,sizeof(recv_buffer));
     	ret = recv(sock_id, recv_buffer, sizeof(recv_buffer), 0);
     	if(ret > 0)
 		{
			recv_hdr = (struct custom_hdr *)&recv_buffer[0];
			if(ntohs(recv_hdr->seq_num) == seq_no)
			{
				if(ntohs(recv_hdr->dst_node) == 01)// check for our packet
				{
					if(ntohs(recv_hdr->src_node) == 02) // file from node_B
					{
						if( (err = write(fd1,&recv_buffer[6],ret-6)) < 0)
						{
							printf("Write Error\n");
							close(sock_id);close(fd1);
							exit(1);
						}
					}
					else if(ntohs(recv_hdr->src_node) == 03) // file from node_C
					{
						if( (err = write(fd2,&recv_buffer[5],ret-5)) < 0)
						{
							printf("Write Error\n");
							close(sock_id);close(fd2);
							exit(1);
						}
					}
					seq_no++;
				}
			}
			else
				continue; // if duplicate is received then drop it and continue...
			
			if(flag == 0)
			{
				flag = 1;
				/* timestamp the start time */
				if(gettimeofday(&tyme1, NULL) < 0)
				{
					printf("Error in calculating the data rate\n");
					exit(1);
				}	
			}
			sum = sum + err;
			if(ret < 1500)
			{
				if((recv_buffer[0] == 0x01) && (recv_buffer[1] == 0x80))
					continue;
				else
					break_cnt++;
			
				if(break_cnt == 2)
					break;
			}
		}
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
	printf("\n%d bytes recieved in %5.2fsecs, data rate = %5.2fkbps\n\n",(sum),(time1/1000000.0),rate);
	return 0;
}