#ifndef LIB
#define LIB

typedef struct {
    int len;
    char payload[1400];
}  __attribute__((packed)) msg;

typedef struct {
	char SOH;
	unsigned char LEN;
	char SEQ;
	char TYPE;
	char DATA[250];
	unsigned short CHECK;
	char MARK;	
} __attribute__((packed)) mini_kermit;

typedef struct {
	char SOH;
	char LEN;
	char SEQ;
	char TYPE;
	char MAXL;
	char TIME;
	char NPAD;
	char PADC;
	char EOL;
	char QCTL; 
	char QBIN;
	char CHKT;
	char REPT;
	char CAPA; 
	char R;
	unsigned short CHECK;
	char MARK;	
} __attribute__((packed)) mini_kermit_S;

void init(char* remote, int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
msg* receive_message_timeout(int timeout); //timeout in milliseconds
unsigned short crc16_ccitt(const void *buf, int len);

#endif

