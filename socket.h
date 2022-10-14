#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

char* get_ip_addr(in_addr_t ip_addr);

#define FAIL				-1
#define ERROR				0
#define OK					1
#define LenTCPbuffer		1000
#define UDP_PORT			3425
#define SERVER_CONN_NUM		1
#define SC_TM_CNTL			60000
#define SOCK_BUF_EMPTY		11/*ошибка чтени§ когда в буфере нет данных*/


#pragma pack(push,1)
typedef struct
{
	int id;
	char tx[10];
	int temp;
	int light;
} msg_t;
#pragma pack(pop)

void ssleep(size_t sec);
void msleep(size_t msec);


int	Connect(void); //Эткрытие server сокета
void Close(void);
int Send(const char* Message, size_t size, const char* addr, uint16_t target_port);
int Recive(void);
void Analys(char* ch, int sz);
