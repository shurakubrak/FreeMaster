#include "socket.h"
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/timeb.h>

extern bool is_master;
extern int ID;

/****************/
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

int Connect(void)
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

	printf("Socket UDP OK\n");
	status = OK;
	return OK;
}
//----------------------------------------------------------

void Close(void)
{
	msleep(100);
	if (UDPSock > -1)
		close(UDPSock);
}
//------------------------------------------------------------

int Send(const char* Message, size_t size, in_addr_t addr, uint16_t target_port)
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

int Recive(void)
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
			Analys(szBuffer, count, from.sin_addr.s_addr);
			//Send("1234"/*buf_answer*/, 4/*1 + sizeof(answer_t)*/, inet_addr("192.168.42.120")/*addr*/, UDP_PORT);
		}
		return OK;
}
//-------------------------------------------------------------------------

void Analys(char* ch, int sz, in_addr_t addr)
{
	msg_t* msg;
	answer_t* answ;
	int32_t* pid;

	switch (ch[0])
	{
	case BLINK:
		if (sz == 1 + sizeof(int32_t)) {
			pid = (int*)(&ch[1]);
			if (ID > *pid) {
				is_master = false;
				Answer(addr);
				printf("Anays is slave\n");
			}
			else if (ID == *pid)
				;// printf("Anays ID == id\n");
			else {
				Answer(addr);
				printf("Anays ID < id\n");
			}
		}
		break;
	case ANSWER:
		if (sz == 1 + sizeof(answer_t)) {
			answ = (answer_t*)(&ch[1]);
			printf("Reciv ANSWER: %d, %d, %d\n", answ->id, answ->temp, answ->light);
		}
		break;
	case MSG:
		if (sz == 1 + sizeof(msg_t)) {
			msg = (msg_t*)(&ch[1]);
			printf("Recived: %d, %s, %d, %d\n", msg->id, msg->tx, msg->temp, msg->light);
		}
		break;
	}
	
	
	return;
}
//--------------------------------------------------------------------------

void Answer(in_addr_t addr)
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
	buf_answer[9] = ((char*)(&Lght))[4];
	buf_answer[10] = ((char*)(&Lght))[5];
	buf_answer[11] = ((char*)(&Lght))[6];
	buf_answer[12] = ((char*)(&Lght))[7];

	Send(buf_answer, 1 + sizeof(answer_t), addr, UDP_PORT);
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