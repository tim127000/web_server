#include "network.h"

Counter counter[MAX_IP];

char *mac_ntoa(u_char *a){
	
	static char line[MAX_ADDLEN];

	sprintf(line , "%02x:%02x:%02x:%02x:%02x:%02xx" , a[0] , a[1] , a[2] , a[3] , a[4] , a[5]);
	return line;
}

void dump_tcp(const u_char *content){

	struct ip *ip = (struct ip*)(content + ETHER_HDR_LEN);
	struct tcphdr *tcp = (struct tcphdr*)(content + ETHER_HDR_LEN + (ip->ip_hl << 2));

	u_int16_t source_port = ntohs(tcp->th_sport);
	u_int16_t destination_port = ntohs(tcp->th_dport);
	
	printf("Protocol : TCP\n");
    	printf("Source Port : %d\n" , source_port);
	printf("Destination Port : %d\n" , destination_port);
}

void dump_udp(const u_char *content){
	
	struct ip *ip = (struct ip*)(content + ETHER_HDR_LEN);
	struct udphdr *udp = (struct udphdr*)(content + ETHER_HDR_LEN + (ip->ip_hl << 2));

	u_int16_t source_port = ntohs(udp->uh_sport);
	u_int16_t destination_port = ntohs(udp->uh_dport);

	printf("Protocol: UDP\n");
	printf("Source Port : %d\n" , source_port);
	printf("Destination Port : %d\n" , destination_port);
}

int main(int argc , char *argv[]){
	
	int i , count = 0;
	char *file_name;

	if(argc != 2){
		fprintf(stderr , "usage:./read_pcap <filename>\n");
		exit(1);
	}
	else{
		file_name = strdup(argv[1]);
	}
	
	char errbuff[PCAP_ERRBUF_SIZE];
	char *dev;

	pcap_t *handler = pcap_open_offline(file_name , errbuff);

	struct pcap_pkthdr *header;
	struct ether_header *eth_header;
	struct ip *ip_header;
	struct tcphdr *tcp_header;
	struct udphdr *udp_header;

	char src_ip[INET_ADDRSTRLEN];
	char dst_ip[INET_ADDRSTRLEN];

	for(i = 0 ; i<MAX_IP ; i++){
		counter[i].num = 0;
		memset(counter[i].src_ip , '\0' , INET_ADDRSTRLEN);
		memset(counter[i].dst_ip , '\0' , INET_ADDRSTRLEN);
	}

	u_char *packet;
	int packet_num = 1;
	u_int size_ip;
	u_int size_tcp;
	time_t raw_time;
	struct tm ts;
	char date[80];
	int res;

	u_char protocol;

	while((res = pcap_next_ex(handler , &header , &packet)) >= 0){
		
		if(res == 0) continue;
		char dst_mac_addr[MAX_ADDLEN] = {};
		char src_mac_addr[MAX_ADDLEN] = {};
		u_int16_t type;
		fprintf(stderr , "Packet %d\n" , packet_num++);

		raw_time = header->ts.tv_sec;
		ts = *localtime(&raw_time);
		strftime(date , sizeof(date) , "%a %Y-%m-%d %H:%M:%s" , &ts);

		printf("Time : %s\n" , date);

		eth_header = (struct ether_header*) packet;

		strcpy(dst_mac_addr , mac_ntoa(eth_header->ether_dhost));
		strcpy(src_mac_addr , mac_ntoa(eth_header->ether_shost));

		printf("Destination Mac Address : %s\n" , dst_mac_addr);
		printf("Source Mac Address : %s\n" , src_mac_addr);

		type = ntohs(eth_header->ether_type);

		if(type == ETHERTYPE_IP){
			
			printf("Type : IP\n");

			ip_header = (struct ip*)(packet + sizeof(struct ether_header));
			inet_ntop(AF_INET , &(ip_header->ip_src) , src_ip , INET_ADDRSTRLEN);
			inet_ntop(AF_INET , &(ip_header->ip_dst) , dst_ip , INET_ADDRSTRLEN);
			protocol = ip_header->ip_p;

			printf("Protocol: IP\n");

			printf("Destination IP : %s\n" , dst_ip);
			printf("Source IP : %s\n" , src_ip);
			
			switch(protocol){
				
				case IPPROTO_UDP:
					dump_udp(packet);
					break;

				case IPPROTO_TCP:
					dump_tcp(packet);
					break;
			}
		}
		else if(type == ETHERTYPE_ARP){
			printf("Type : ARP\n");
		}
		else if(type == ETHERTYPE_REVARP){
			
			printf("Type : Reverse ARP\n");
		}
		else{
			printf("Not support!\n");
		}

		printf("-----------------------------------------\n");
	}
}
