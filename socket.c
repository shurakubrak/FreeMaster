#include "socket.h"
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/timeb.h>

extern bool is_master;
extern int id;

/****************/

int UDPSock = FAIL;
struct sockaddr_in local_addr;
int status;
unsigned int port = 6030;
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

int Send(const char* Message, size_t size, const char* addr, uint16_t target_port)
{
	struct sockaddr_in target;
	target.sin_addr.s_addr = inet_addr(addr);
	if (target.sin_addr.s_addr <= 0) {
		perror("socket send error: ");
		return ERROR;
	}

	if (target_port == FAIL)
		target.sin_port = htons(port);/*порт приёма*/
	else
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
		int count = recv(UDPSock, szBuffer, LenTCPbuffer, MSG_DONTWAIT);
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
			Analys(szBuffer, count);
		}
		return OK;
}
//-------------------------------------------------------------------------

void Analys(char* ch, int sz)
{
	msg_t* msg;
	msg = (msg_t*)ch;
	printf("Recived: %d, %s, %d, %d\n", msg->id, msg->tx, msg->temp, msg->light);
	if (id > msg->id) {
		is_master = false;
		printf("is slave\n");
	}
	return;
}
//--------------------------------------------------------------------------

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