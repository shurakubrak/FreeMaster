#include "socket.h"
#include <stdio.h>
#include <errno.h>
#include <sys/timeb.h>
#include <netdb.h>

extern bool is_master;
extern int ID;

/****************/
devs_t Devs[DEVS_COUNT];
pthread_mutex_t mutex;
int Tmp;
int Lght;
int UDPSock = FAIL;
struct sockaddr_in local_addr;
int status;
unsigned int port = UDP_PORT;

char buf_answer[sizeof(answer_t) + 1] = { ANSWER,0,0,0,0,0,0,0,0,0,0,0,0 };
/*****************************/

char* get_ip_addr(in_addr_t ip_addr)
{
	unsigned long s_addr = ip_addr;
	//return inet_ntoa(s_addr);
}

void ssleep(size_t sec)
{
	usleep(sec * 1000000);
}
void msleep(size_t msec)
{
	usleep(msec * 1000);
}

///////// SocketUDP //////////////////////////////////////////////////////////

uint64_t get_time_ms()
{
	struct timeb It;
	ftime(&It);
	return (It.time*1000) + It.millitm;
}

//-----------------------------------------------------------------

int s_connect(void)
{
	if (UDPSock > -1) {
		close(UDPSock);
		status = ERROR;
	}

	UDPSock = socket(AF_INET, SOCK_DGRAM, 17);/*'17' -протокол UDP*/
	if (UDPSock < 0) {
		printf("Socket UDP init error\n");
		return ERROR;
	}

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(port);
	
	int err = bind(UDPSock, (struct sockaddr*)&local_addr, sizeof(local_addr));
	if (err < 0) {
		perror("bind");
		printf("Socket UDP create error\n");
		status = ERROR;
		return ERROR;
	}
	
	//char hostname[26];
	//struct addrinfo* result;
	//int s = getaddrinfo(NULL, hostname, &local_addr, &result);
	//if (s != 0) 
	//	perror("ee: ");
	status = OK;
	return OK;
}
//----------------------------------------------------------

void s_close(void)
{
	msleep(100);
	if (UDPSock > -1)
		close(UDPSock);
}
//------------------------------------------------------------

int s_send(const char* Message, size_t size, in_addr_t addr, uint16_t target_port)
{
	struct sockaddr_in target;
	target.sin_addr.s_addr = addr;// inet_addr(addr);
	if (target.sin_addr.s_addr <= 0) {
		perror("socket send error: ");
		return ERROR;
	}
	target.sin_family = AF_INET;
	target.sin_port = htons(target_port);

	int err = sendto(UDPSock, Message, size, 0, &target, sizeof(target));
	if (err < 0) {
		perror("socket send error: ");
		//Close();
		return ERROR;
	}
	return OK;
}
//----------------------------------------------------------------------------

int s_recive(void)
{
	char szBuffer[LenTCPbuffer];
	struct sockaddr_in from;
	socklen_t fromLength = sizeof(from);

		int count = recvfrom(UDPSock, szBuffer, LenTCPbuffer, MSG_DONTWAIT,
			&from, &fromLength);
		if (count < 0) {
			if (errno != SOCK_BUF_EMPTY) {
				perror("socket read error: ");
				return FAIL;
			}
			else
				return SOCK_BUF_EMPTY;
		}

		else if (count == 0) {
			perror("read 0 bytes, connection will be destrow");
			return FAIL;
		}
		else if (count < 1001) {
			szBuffer[count] = 0;
			s_analys(szBuffer, count, from.sin_addr.s_addr);
		}
		return OK;
}
//-------------------------------------------------------------------------

void s_analys(char* ch, int sz, in_addr_t addr)
{
	msg_t* msg;
	answer_t* answ;
	devs_t dev;
	int32_t* pid;
	
	switch (ch[0])
	{
	case BLINK:
		/*получили от мастера маяк.
		если его id меньше, то снимаем статус мастера
		иначе статус не меняется.
		отвечаем только если мы не мастер*/
		if (sz == 1 + sizeof(int32_t)) {
			pid = (int*)(&ch[1]);
			if (ID > *pid) {
				if (is_master) {
					is_master = false;
					printf("is slave\n");
				}
				answer(addr);
			}
			else if (ID == *pid)
				;// сам себе;
			else if(!is_master)
				answer(addr);
		}
		break;
	case ANSWER:
		/*получили ответ на маяк
		если мастер, то обрабатываем*/
		if(is_master)
			if (sz == 1 + sizeof(answer_t)) {
				answ = (answer_t*)(&ch[1]);
				//printf("Reciv ANSWER: %d, %d, %d\n", answ->id, answ->temp, answ->light);
				dev.id = answ->id;
				dev.addr = addr;
				dev.active = true;
				dev.temp[0] = answ->temp;
				dev.light[0] = answ->light;
				devs_access(&dev, UPD_PAR);
			}
		break;
	case MSG:
		/*получили сообщение от мастера
		выводим на экран*/
		if (sz == 1 + sizeof(msg_t)) {
			msg = (msg_t*)(&ch[1]);
			printf("Recived message: %s, %d, %d\n", msg->tx, msg->temp, msg->light);
		}
		break;
	}
	
	
	return;
}
//--------------------------------------------------------------------------

void answer(in_addr_t addr)
{
	/*установим фиксированные параметры для простоты*/
	Tmp = ID / 10;
	Lght = ID * 10;

	buf_answer[1] = ((char*)(&ID))[0];
	buf_answer[2] = ((char*)(&ID))[1];
	buf_answer[3] = ((char*)(&ID))[2];
	buf_answer[4] = ((char*)(&ID))[3];
	buf_answer[5] = ((char*)(&Tmp))[0];
	buf_answer[6] = ((char*)(&Tmp))[1];
	buf_answer[7] = ((char*)(&Tmp))[2];
	buf_answer[8] = ((char*)(&Tmp))[3];
	buf_answer[9] = ((char*)(&Lght))[0];
	buf_answer[10] = ((char*)(&Lght))[1];
	buf_answer[11] = ((char*)(&Lght))[2];
	buf_answer[12] = ((char*)(&Lght))[3];

	s_send(buf_answer, 1 + sizeof(answer_t), addr, UDP_PORT);
}
//-----------------------------------------------------------------------

int get_network()
{
	FILE* fd;
	char buf[12];
	
	fd = fopen("idd", "r");
	if (fd != NULL) {
		if(fgets(buf, sizeof(buf), fd) != NULL){
			return atoi(buf);
		}
	}
	fclose(fd);
	return FAIL;
}
//-----------------------------------------------------------------

void devs_init(in_addr_t start_addr)
{
	for (size_t d = 0; d < DEVS_COUNT; d++) {
		Devs[d].id = ERROR;
		Devs[d].addr = start_addr + d;
		Devs[d].active = false;
		Devs[d].count = ERROR;
		for (size_t i = 0; i < MEAS_COUNT; i++) {
			Devs[d].temp[i] = ERROR;
			Devs[d].light[i] = ERROR;
		}
		Devs[d].tm = get_time_ms();
	}
	/*инициализация блокировки*/
	pthread_mutex_init(&mutex, NULL);
}
//---------------------------------------------------------

bool devs_access(devs_t* devs, acc_t acc)
{
	size_t cd;
	bool ret = false;
	 
	/*блокируем*/
	pthread_mutex_lock(&mutex);
	switch (acc)
	{
	case UPD_ST:
		cd = devs_find(htonl(devs->addr));
		if (cd != FAIL) {
			Devs[cd].active = devs->active;
			Devs[cd].tm = get_time_ms();
		}
		break;
	case UPD_PAR:
		cd = devs_find(htonl(devs->addr));
		if (cd != FAIL) {
			if (Devs[cd].count >= MEAS_COUNT)
				Devs[cd].count = 0;
			Devs[cd].active = devs->active;
			Devs[cd].temp[Devs[cd].count] = devs->temp[0];
			Devs[cd].light[Devs[cd].count] = devs->light[0];
			Devs[cd].count++;
			Devs[cd].tm = get_time_ms();
			ret = true;
		}
		break;
	case READ_MIDL:
		if (devs->id < 0 || devs->id > DEVS_COUNT)
			break;

		cd = devs->id;
		if (!Devs[cd].active)
			break;
		devs->addr = ntohl(Devs[cd].addr);
		devs->temp[0] = 0;
		devs->light[0] = 0;
		for (size_t i = 0; i < MEAS_COUNT; i++) {
			devs->temp[0] += Devs[cd].temp[i];
			devs->light[0] += Devs[cd].light[i];
		}
		devs->temp[0] /= MEAS_COUNT;
		devs->light[0] /= MEAS_COUNT;
		ret = true;
		break;
	case CLEAR:
		cd = devs->id;
		if (!Devs[cd].active)
			break;
		if (get_time_ms() - Devs[cd].tm >= TM_CLEAR) {
			Devs[cd].id = ERROR;
			Devs[cd].active = false;
			Devs[cd].count = ERROR;
			for (size_t i = 0; i < MEAS_COUNT; i++) {
				Devs[cd].temp[i] = ERROR;
				Devs[cd].light[i] = ERROR;
			}
			Devs[cd].tm = get_time_ms();
			ret = true;
		}
		break;
	}
	/*снимаем блокировку*/
	pthread_mutex_unlock(&mutex);
	return ret;
}
//-----------------------------------------------

size_t devs_find(in_addr_t addr)
{
	for (size_t i = 0; i < DEVS_COUNT; i++)
		if (Devs[i].addr == addr)
			return i;
	return FAIL;
}