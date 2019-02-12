#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc, char** argv) {
    	msg t;
	msg *y;
	mini_kermit p;
	mini_kermit_S s;
	int fd, seq = 0, m = 0;
	char* file = (char *) malloc(15*sizeof(char));
	char buffer[250];
    	init(HOST, PORT);

	while(m <= 3) {
		y = receive_message_timeout(5000);
		if(m == 3)
			exit(0);
		if(y == NULL) {
			printf("[%s] TIMEOUT S\n", argv[0]);
			m++;
		}
		else {
			s = *((mini_kermit_S *)y->payload);
			if(crc16_ccitt(&s, sizeof(s) - 4) == s.CHECK && s.SEQ == seq) {
				printf("[%s] ACK for S\n", argv[0]);
				p.TYPE = 'Y';	
			}
			else {
				printf("[%s] NAK for S\n", argv[0]);
				p.TYPE = 'N';
			}
				
			p.SOH = 1;
			p.SEQ = seq + 1;
			p.CHECK = 0;
			p.MARK = 13;
			if(p.TYPE == 'Y') {
			p.LEN = 16;	
				memset(p.DATA, 0, sizeof(p.DATA));
				memcpy(p.DATA, &(s.MAXL), 11);
			}
			else {			
				p.LEN = 5;		
				memset(p.DATA, 0, sizeof(p.DATA));
			}
	
			t.len = p.LEN + 2;
			memset(t.payload, 0, sizeof(t.payload));
			memcpy(t.payload, &p, t.len);	
				
			seq = seq + 2;
			m = 0;

			send_message(&t);
			if(p.TYPE == 'Y')
				break;
		}
	}
	while(1) {
	while(m <= 3) {
		y = receive_message_timeout(5000);
		if(m == 3)
			exit(0);
		if(y == NULL) {
			printf("[%s] TIMEOUT\n", argv[0]);
			m++;
		}
		else {
			p = *((mini_kermit *)y->payload);
			if(crc16_ccitt(&p, sizeof(p) - 4) == p.CHECK && p.SEQ == seq) {
				printf("[%s] ACK for %c\n", argv[0], p.TYPE);
				if(p.TYPE == 'F') { 
					sprintf(file,"recv_%s",p.DATA);
					fd = open(file,O_WRONLY | O_CREAT, 0644);
				} 
				if(p.TYPE == 'D') { 
					memset(buffer, 0, sizeof(buffer));
					memcpy(buffer, p.DATA, p.LEN - 5);
					write(fd, buffer, p.LEN - 5);
				}
				if(p.TYPE == 'Z') {
					close(fd);
				}
				p.TYPE = 'Y';	
				
			}
			else {
				printf("[%s] NAK for %c\n", argv[0], p.TYPE);
				p.TYPE = 'N';
			}
				
			p.SOH = 1;
			p.SEQ = seq + 1;
			p.CHECK = 0;
			p.MARK = 13;
			p.LEN = 5;		
			memset(p.DATA, 0, sizeof(p.DATA));
	
			t.len = p.LEN + 2;
			memset(t.payload, 0, sizeof(t.payload));
			memcpy(t.payload, &p, t.len);	
				
			seq = (seq + 2) % 64;
			m = 0;

			send_message(&t);
			if(p.TYPE == 'Y')
				break;
		}
	}
	p = *((mini_kermit *)y->payload);
	if(p.TYPE == 'B')
		break;
	}
	return 0;
}
