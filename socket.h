#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>

char* get_ip_addr(in_addr_t ip_addr);

#define FAIL				-1
#define ERROR				0
#define OK					1
#define LenTCPbuffer		1000
#define UDP_PORT			6030
#define SOCK_BUF_EMPTY		11/*ошибка чтени§ когда в буфере нет данных*/

typedef enum 
{
	UPD_PAR = 0,
	READ_MIDL,
	CLEAR,
	UPD_ST
}acc_t;

#define DEVS_COUNT			200
#define MEAS_COUNT			10
#define TM_CLEAR			6000

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

typedef struct
{
	int32_t id;
	in_addr_t addr;
	bool active;
	int32_t temp[MEAS_COUNT];
	int32_t light[MEAS_COUNT];
	uint8_t count;
	uint64_t tm;
}devs_t;

void ssleep(size_t sec);
void msleep(size_t msec);


int	s_connect(void); //открытие server сокета
void s_close(void);
int s_send(const char* Message, size_t size, in_addr_t addr, uint16_t target_port);
int s_recive(void);
void s_analys(char* ch, int sz, in_addr_t addr);
void answer(in_addr_t addr);
void devs_init(in_addr_t start_addr);
bool devs_access(devs_t* devs, acc_t acc);
size_t devs_find(in_addr_t addr);
