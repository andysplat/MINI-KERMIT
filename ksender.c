#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc, char** argv) {
	msg t;
	msg *y;
	mini_kermit p;
	mini_kermit_S s;
	int fd, count, seq = 0, n = 0, m = 0;
	char buffer[250];
	init(HOST, PORT);

	s.SOH = 1;
	s.LEN = 16;
	s.SEQ = seq;
	s.TYPE = 'S';
	s.MAXL = 250;
	s.TIME = 5;
	s.NPAD = 0;
	s.PADC = 0;
	s.EOL = 13;
	s.QCTL = 0; 
	s.QBIN = 0;
	s.CHKT = 0;
	s.REPT = 0;   
	s.CAPA = 0; 
	s.R = 0;
	s.CHECK = crc16_ccitt(&s, sizeof(s) - 4);
	s.MARK = 13;

	t.len = s.LEN + 2;
	memset(t.payload, 0, sizeof(t.payload));
	memcpy(t.payload, &s, t.len);

	send_message(&t); 
	while(m<=3) {
		if(m == 3)
			exit(0);
		else {
			y = receive_message_timeout(5000);
    			if (y == NULL) {
        			printf("[%s] TIMEOUT S\n", argv[0]);
				m++;
				send_message(&t);
    			} 
			else {
				p = *((mini_kermit *)y->payload);
				m = 0;
				seq = seq + 2;
				if(p.TYPE == 'Y' && p.SEQ == seq - 1)  {
					printf("[%s] ACK for S\n", argv[0]);
					break;
				}
				else {
					printf("[%s] NAK for S\n", argv[0]);
					s = *((mini_kermit_S *)t.payload);
					s.SEQ = seq;
					s.CHECK = crc16_ccitt(&s, sizeof(s) - 4);
					t.len = s.LEN + 2;
					memset(t.payload, 0, sizeof(t.payload));
					memcpy(t.payload, &s, t.len);
					send_message(&t);
				}
    			}
		}
	}
	
	for(n = 1; n < argc; n++) {
		
		p.SOH = 1;		
		p.LEN = strlen(argv[n]) + 5;
		p.TYPE = 'F';
		p.SEQ = seq;
		memset(p.DATA, 0, sizeof(p.DATA));
		memcpy(p.DATA, argv[n], strlen(argv[n]));
		p.CHECK = crc16_ccitt(&p, sizeof(p) - 4);
		p.MARK = 13;
		
		t.len = p.LEN + 2;
		memset(t.payload, 0, sizeof(t.payload));
		memcpy(t.payload, &p, sizeof(p));		
		send_message(&t);
		
		
		while(m <= 3) {
			if(m == 3)
				exit(0);
			else {
				y = receive_message_timeout(5000);
    				if (y == NULL) {
        				printf("[%s] TIMEOUT F\n", argv[0]);
					m++;
					send_message(&t);
    				} 
				else {
					p = *((mini_kermit *)y->payload);
					m = 0;	
					seq = seq + 2;
					if(p.TYPE == 'Y' && p.SEQ == seq - 1) {
						printf("[%s] ACK for F\n", argv[0]);
						seq = (seq) % 64;
						break;
					}
					else {
       						printf("[%s] NAK for F\n", argv[0]);
						seq = (seq) % 64;
						p = *((mini_kermit *)t.payload);
						p.SEQ = seq;
						p.CHECK = crc16_ccitt(&p, sizeof(p) - 4);
						t.len = p.LEN + 2;
						memset(t.payload, 0, sizeof(t.payload));
						memcpy(t.payload, &p, sizeof(p));
						send_message(&t);
					}
    				}
			}
		}
		
		fd = open(argv[n], O_RDONLY);		
		while ((count = read(fd, buffer, 250)) > 0) { 
			
			p.SOH = 1;	
			p.LEN = count + 5;
			p.TYPE = 'D';
			p.SEQ = seq;
			memset(p.DATA, 0, sizeof(p.DATA));
			memcpy(p.DATA, buffer, count);
			p.CHECK = crc16_ccitt(&p, sizeof(p) - 4);
			p.MARK = 13;

			t.len = p.LEN + 2;
			memset(t.payload, 0, sizeof(t.payload));
			memcpy(t.payload, &p, sizeof(p));		
			send_message(&t);
			
			while(m <= 3) {
				if(m == 3)
					exit(0);
				else {
					y = receive_message_timeout(5000);
    					if (y == NULL) {
        					printf("[%s] TIMEOUT D\n", argv[0]);
						m++;
						send_message(&t);
    					} 
					else {
						p = *((mini_kermit *)y->payload);
						m = 0;	
						seq = seq + 2;
						if(p.TYPE == 'Y' && (p.SEQ == seq - 1)) {
							printf("[%s] ACK for D\n", argv[0]);
							seq = (seq) % 64;
							break;
						}
						else {
       							printf("[%s] NAK for D\n", argv[0]);
							seq = (seq) % 64;
							p = *((mini_kermit *)t.payload);
							p.SEQ = seq;
							p.CHECK = crc16_ccitt(&p, sizeof(p) - 4);
							t.len = p.LEN + 2;
							memset(t.payload, 0, sizeof(t.payload));
							memcpy(t.payload, &p, sizeof(p));
							send_message(&t);
						}
    					}
				}
			}
		}
		
		p.SOH = 1;
		p.LEN = 5;
		p.TYPE = 'Z';
		p.SEQ = seq;
		memset(p.DATA, 0, sizeof(p.DATA));
		p.CHECK = crc16_ccitt(&p, sizeof(p) - 4);
		p.MARK = 13;

		t.len = p.LEN + 2;
		memset(t.payload, 0, sizeof(t.payload));
		memcpy(t.payload, &p, sizeof(p));	
		send_message(&t);
		
		while(m <= 3) {
			if(m == 3)
				exit(0);
			else {
				y = receive_message_timeout(5000);
    				if (y == NULL) {
        				printf("[%s] TIMEOUT Z\n", argv[0]);
					m++;
					send_message(&t);
    				} 
				else {
					p = *((mini_kermit *)y->payload);
					m = 0;	
					seq = seq + 2;
					if(p.TYPE == 'Y' && p.SEQ == seq - 1) {
						printf("[%s] ACK for Z\n", argv[0]);
						seq = (seq) % 64;
						break;
					}
					else {
       						printf("[%s] NAK for Z\n", argv[0]);
						seq = (seq) % 64;
						p = *((mini_kermit *)t.payload);
						p.SEQ = seq;
						p.CHECK = crc16_ccitt(&p, sizeof(p) - 4);
						t.len = p.LEN + 2;
						memset(t.payload, 0, sizeof(t.payload));
						memcpy(t.payload, &p, sizeof(p));
						send_message(&t);
					}
    				}
			}
		}
		close(fd);	
	}		
		
	p.SOH = 1;
	p.LEN = 5;
	p.TYPE = 'B';
	p.SEQ = seq;
	memset(p.DATA, 0, sizeof(p.DATA));
	p.CHECK = crc16_ccitt(&p, sizeof(p) - 4);
	p.MARK = 13;

	t.len = p.LEN + 2;
	memset(t.payload, 0, sizeof(t.payload));
	memcpy(t.payload, &p, sizeof(p));
	send_message(&t);

	while(m <= 3) {
		if(m == 3)
			exit(0);
		else {
			y = receive_message_timeout(5000);
    			if (y == NULL) {
       				printf("[%s] TIMEOUT B\n", argv[0]);
				m++;
				send_message(&t);
    			} 
			else {
				p = *((mini_kermit *)y->payload);
				m = 0;	
				seq = seq + 2;
				if(p.TYPE == 'Y' && p.SEQ == seq - 1) {
					printf("[%s] ACK for B\n", argv[0]);
					seq = (seq) % 64;
					break;
				}
				else {
       					printf("[%s] NAK for B\n", argv[0]);
					seq = (seq) % 64;
					p = *((mini_kermit *)t.payload);
					p.SEQ = seq;
					p.CHECK = crc16_ccitt(&p, sizeof(p) - 4);
					t.len = p.LEN + 2;
					memset(t.payload, 0, sizeof(t.payload));
					memcpy(t.payload, &p, sizeof(p));
					send_message(&t);
				}
    			}
		}
	}
		
	return 0;
}
