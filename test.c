/*-----------------------------------------------------------------------------
Filename:- 	test.c
Descritption:- This is the main C file for the custom network project 
			created as part of CSCI558L lab project.
Date:-		Oct 11th 2014 - 
Authors:- 	Lohith Bellad,Weichen Zhao
			University of Southern California, Los Angeles, California
Platform:- 	Mac OS X Mountain Lion, Ubuntu 12.04
Place:-		Los Angeles, California
-----------------------------------------------------------------------------*/
#include "custom.h"

int main(int argc, char **argv)
{
   int handle = 0,ret=0;
   uint8_t* mac;
   u_char dst_mac[3],*ptrr;
   struct ifreq ifr;
   u_char buffer[2048];
   struct ifaddrs *addrs,*tmp;
   int s, intfCount = 0;
   char host[NI_MAXHOST], mask[NI_MAXHOST];
   struct ll_addr* intfSockTable;
   int tableIndex = 0;
   struct sockaddr_ll sll;
   int err,sending_sock,t_size;
   struct routingTableElem route_table[50];//creating table for large number of networks
   unsigned short node_ad;
  
	   
   if(argc < 1)
   {
	   printf("Usage: sudo ./route eth#");
	   return 0;
   }
   
   // create raw socket that listens to everything
   handle = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
   memset(&ifr, 0, sizeof(ifr));
   memset(&sll, 0, sizeof(sll));
   sll.sll_family    = AF_PACKET;
   sll.sll_protocol  = htons(ETH_P_ALL);//htons(0x3333)
   if ( bind( handle, (struct sockaddr *) &sll, sizeof(sll)) == -1 )
   {
      printf("fail binding\n");
      return 0;
   }
   printf("Binding listening socket to %x\n", sll.sll_protocol);

   getifaddrs(&addrs);
   tmp = addrs;
   //checking the number of interfaces
   while (tmp)
   {
       if (tmp->ifa_addr->sa_family == AF_INET)
			intfCount += 1;
		tmp = tmp->ifa_next;
   }
   intfCount -= 2;
   printf("----------------------------------------------------------------------------------------------------------\n");
   printf("Total number of custom routing interfaces is: %d\n",intfCount);
   printf("------------------------------------------------\n");
   printf("Name\t    IP\t\t  SUBNET MASK\t\t    MAC Address\n");
   printf("----\t    --\t\t  -----------\t\t    -----------\n");
   // allocation memory for intf <-> socket table
   intfSockTable = malloc(intfCount * sizeof(struct ll_addr));
   ptrr = dst_mac;
   tmp = addrs;
   while (tmp)
   {
   		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
       	   printf("%s", tmp->ifa_name);
       	   //family = tmp->ifa_addr->sa_family;

       	   s = getnameinfo(tmp->ifa_addr, sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
             s |= getnameinfo(tmp->ifa_netmask, sizeof(struct sockaddr_in),mask, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
             if (s != 0) 
	  	   {
          	  printf("getnameinfo() failed: %s\n", gai_strerror(s));
           	  exit(EXIT_FAILURE);
	  	   }
       	   printf("\t<%s>\t <%s>", host, mask);
	  	   strcpy( ifr.ifr_name, tmp->ifa_name );
		   if( ioctl( handle, SIOCGIFHWADDR, &ifr ) != -1 )
		   {
			   if(strncmp(tmp->ifa_name,"lo",2))
		        {
			   	mac = (uint8_t*)ifr.ifr_ifru.ifru_hwaddr.sa_data;
		        	printf( "\t<%02X:%02X:%02X:%02X:%02X:%02X>\n",MAC_ADDR(mac));
			   }
			   else
				   printf("\t\t\t---\n");
			   if( isRoutingPort(tmp->ifa_name,argv[1]) )
			   {
				   intfSockTable[tableIndex].sock_id = createSocket(tmp->ifa_name);
			 	   intfSockTable[tableIndex].if_ad = tmp;
				   tableIndex += 1;
			  	   sprintf((char *)ptrr,"%c",mac[5]);
				   ptrr = ptrr+1;
			   }
       	   }
		}
    	 	tmp = tmp->ifa_next;
    }
      
    // Adding static routing table entries to the table, depending on interfaces used for custom routing 
    err = routingTableAddEntry(route_table, (unsigned short)atoi(argv[2]),intfSockTable[0].if_ad->ifa_name,intfSockTable[0].sock_id,1, &t_size);  
    err = routingTableAddEntry(route_table, (unsigned short)atoi(argv[3]),intfSockTable[1].if_ad->ifa_name,intfSockTable[1].sock_id,1, &t_size); 
    //err = routingTableAddEntry(route_table, (unsigned short)atoi(argv[4]),intfSockTable[2].if_ad->ifa_name,intfSockTable[2].sock_id,1, &t_size); 
        
    err = routingTableDump(route_table, t_size);
    
   // Forever loop!!!, capture packets and forward it, go packets gooooo 
   while(1)
   {
      	memset(&buffer, 0, sizeof(buffer));
      	ret = recv(handle, buffer, sizeof(buffer), 0);
      	if(ret > 0)
	 	{
			 if( (buffer[1] == 0x01 || buffer[1] == 0x02 || buffer[1] == 0x03) && (buffer[4] == 0x01))
		    	{
			    /*-------------------------------the main routing part-------------------------------*/
			    //Looking up the dest_ip next hop MAC
			    node_ad = (unsigned short)ntohs(*((uint16_t *)&buffer[0]));
				if( (err = routingTableLookUp(route_table, node_ad, &t_size ,&sending_sock )) < 0)
				{
					printf("Error looking up the node address\n");
					continue;// if error in looking up, drop the packet and continue with the next packet
				}
				buffer[4] = 0x00;
				
				if( (err = send(sending_sock,buffer,ret,0)) != ret)
				{
					printf("Error in sending(routing) the packet\n");
					continue;// if error in sending the packet, drop it and continue with the next packet
				} 
				/*--------------------------------------------------------------------*/
				// Printing the packet recieved info, comment it if not required, thank you	
	    			/*printf("Source Node Address: %d",ntohs(*((uint16_t *)&buffer[2])));
	    			printf("\tDestination Node Address: %d",ntohs(*((uint16_t *)&buffer[0])));
				printf("\t hop count: %d\n",(int)buffer[4]);*/
			}
	 	}
   }
   freeifaddrs(addrs);
   return 0;
}