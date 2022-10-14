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
#define UDP_PORT			6030
#define SERVER_CONN_NUM		1
#define SC_TM_CNTL			60000
#define SOCK_BUF_EMPTY		11/*ошибка чтени§ когда в буфере нет данных*/

#define BLINK				0x01
#define ANSWER				0x02
#define MSG					0x03


#pragma pack(push,1)
typedef struct
{
	int32_t id;
	char tx[10];
	int32_t temp;
	int32_t light;
} msg_t;

typedef struct
{
	int32_t id;
	int32_t temp;
	int32_t light;
}answer_t;
#pragma pack(pop)

void ssleep(size_t sec);
void msleep(size_t msec);


int	Connect(void); //Эткрытие server сокета
void Close(void);
int Send(const char* Message, size_t size, in_addr_t addr, uint16_t target_port);
int Recive(void);
void Analys(char* ch, int sz, in_addr_t addr);
void Answer(in_addr_t addr);

